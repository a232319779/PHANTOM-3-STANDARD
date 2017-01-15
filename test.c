/*
 * Copyright 2012 Jared Boone <jared@sharebrained.com>
 * Copyright 2013 Benjamin Vernoux <titanmkd@gmail.com>
 * Copyright 2013 Michael Ossmann <mike@ossmann.com>
 *
 * This file is part of HackRF.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <hackrf.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <signal.h>

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#define strtoull _strtoui64
#ifndef snprintf
#define snprintf _snprintf
#endif


#define FD_BUFFER_SIZE (8*1024)

#define FREQ_ONE_MHZ (1000000ull)

#define DEFAULT_FREQ_HZ (900000000ull) /* 900MHz */
#define FREQ_MIN_HZ	(0ull) /* 0 Hz */
#define FREQ_MAX_HZ	(7250000000ull) /* 7250MHz */
#define IF_MIN_HZ (2150000000ull)
#define IF_MAX_HZ (2750000000ull)
#define LO_MIN_HZ (84375000ull)
#define LO_MAX_HZ (5400000000ull)
#define DEFAULT_LO_HZ (1000000000ull)

#define DEFAULT_SAMPLE_RATE_HZ (10000000) /* 10MHz default sample rate */

#define DEFAULT_BASEBAND_FILTER_BANDWIDTH (5000000) /* 5MHz default */

#define SAMPLES_TO_XFER_MAX (0x8000000000000000ull) /* Max value */

#define BASEBAND_FILTER_BW_MIN (1750000)  /* 1.75 MHz min value */
#define BASEBAND_FILTER_BW_MAX (28000000) /* 28 MHz max value */

#if defined _WIN32
	#define sleep(a) Sleep( (a*1000) )
#endif

#define U64TOA_MAX_DIGIT (31)
typedef struct
{
    char data[U64TOA_MAX_DIGIT+1];
} t_u64toa;

t_u64toa ascii_u64_data1;
t_u64toa ascii_u64_data2;


int print_hackrf_info(hackrf_device_list_t *list)
{
    int result = HACKRF_SUCCESS;
	uint8_t board_id = BOARD_ID_INVALID;
	char version[255 + 1];
	read_partid_serialno_t read_partid_serialno;
	hackrf_device* device;
    int i;
    
	for (i = 0; i < list->devicecount; i++) {
		if (i > 0)
			printf("\n");
			
		printf("Found HackRF board %d:\n", i);
		
		if (list->serial_numbers[i])
			printf("USB descriptor string: %s\n", list->serial_numbers[i]);

		device = NULL;
		result = hackrf_device_list_open(list, i, &device);
		if (result != HACKRF_SUCCESS) {
			fprintf(stderr, "hackrf_open() failed: %s (%d)\n",
					hackrf_error_name(result), result);
			return EXIT_FAILURE;
		}

		result = hackrf_board_id_read(device, &board_id);
		if (result != HACKRF_SUCCESS) {
			fprintf(stderr, "hackrf_board_id_read() failed: %s (%d)\n",
					hackrf_error_name(result), result);
			return EXIT_FAILURE;
		}
		printf("Board ID Number: %d (%s)\n", board_id,
				hackrf_board_id_name(board_id));

		result = hackrf_version_string_read(device, &version[0], 255);
		if (result != HACKRF_SUCCESS) {
			fprintf(stderr, "hackrf_version_string_read() failed: %s (%d)\n",
					hackrf_error_name(result), result);
			return EXIT_FAILURE;
		}
		printf("Firmware Version: %s\n", version);

		result = hackrf_board_partid_serialno_read(device, &read_partid_serialno);	
		if (result != HACKRF_SUCCESS) {
			fprintf(stderr, "hackrf_board_partid_serialno_read() failed: %s (%d)\n",
					hackrf_error_name(result), result);
			return EXIT_FAILURE;
		}
		printf("Part ID Number: 0x%08x 0x%08x\n", 
					read_partid_serialno.part_id[0],
					read_partid_serialno.part_id[1]);
		printf("Serial Number: 0x%08x 0x%08x 0x%08x 0x%08x\n", 
					read_partid_serialno.serial_no[0],
					read_partid_serialno.serial_no[1],
					read_partid_serialno.serial_no[2],
					read_partid_serialno.serial_no[3]);
		
		result = hackrf_close(device);
		if (result != HACKRF_SUCCESS) {
			fprintf(stderr, "hackrf_close() failed: %s (%d)\n",
					hackrf_error_name(result), result);
		}
    }

    return EXIT_SUCCESS;
}

// transfer
bool receive = false;
static volatile bool do_exit = false;

static char *stringrev(char *str)
{
	char *p1, *p2;

	if(! str || ! *str)
		return str;

	for(p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}

char* u64toa(uint64_t val, t_u64toa* str)
{
	#define BASE (10ull) /* Base10 by default */
	uint64_t sum;
	int pos;
	int digit;
	int max_len;
	char* res;

	sum = val;
	max_len = U64TOA_MAX_DIGIT;
	pos = 0;

	do
	{
		digit = (sum % BASE);
		str->data[pos] = digit + '0';
		pos++;

		sum /= BASE;
	}while( (sum>0) && (pos < max_len) );

	if( (pos == max_len) && (sum>0) )
		return NULL;

	str->data[pos] = '\0';
	res = stringrev(str->data);

	return res;
}

static void usage(){
    printf("Usage : \n");
    printf("\t-r <filename> # Receive data into file (use '-' for stdout).\n");
    printf("\t[-f freq_hz] # Frequency in Hz [%sMHz to %s MHz].\n",
            u64toa((FREQ_MIN_HZ/FREQ_ONE_MHZ), &ascii_u64_data1),
            u64toa((FREQ_MAX_HZ/FREQ_ONE_MHZ), &ascii_u64_data2));
    printf("\t[-m image_reject] # Image rejection filter selection, 0=bypass, 1=low pass, 2=high pass.\n");
    printf("\t[-a amp_enable] # RX/TX RF amplifier 1=Enable, 0=Disable.\n");
    printf("\t[-l gain_db] # RX LNA (IF) gain, 0-40dB, 8dB steps\n");
    printf("\t[-g gain_db] # RX VGA (baseband) gain, 0-62dB, 2dB steps\n");
    printf("\t[-x gain_db] # TX VGA (IF) gain, 0-47dB, 1dB steps\n");
    printf("\t[-s sample_rate_hz] # Sample rate in Hz (4/8/10/12.5/16/20MHz, default %sMHz).\n",
            u64toa(DEFAULT_SAMPLE_RATE_HZ/FREQ_ONE_MHZ,&ascii_u64_data1));
    printf("\t[-n num_samples] # Number of samples to transer (default is unlimited).\n");
    printf("\t[-b baseband_filter_bw_hz] # Set baseband filter bandwidth in Hz.\n\tPossible values:1.75/2.5/3.5/5/5.5/6/7/8/9/10/12/14/15/20/24/28HHz,default < sample_rate_hz.\n");
}

void sigint_callback_handler(int signum)
{
    fprintf(stdout, "Caught signal %d\n", signum);
    do_exit = true;
}



int main(int argc, char** argv)
{
	int result = HACKRF_SUCCESS;
	hackrf_device_list_t *list;
    // transfer
    int opt;
    const char* path = NULL;
    
	result = hackrf_init();
	if (result != HACKRF_SUCCESS) {
		fprintf(stderr, "hackrf_init() failed: %s (%d)\n",
				hackrf_error_name(result), result);
		return EXIT_FAILURE;
	}
	list = hackrf_device_list();	
	if (list->devicecount < 1 ) {
		printf("No HackRF boards found.\n");
		return EXIT_FAILURE;
	}
    // print hackrf info.
    print_hackrf_info(list);

    // parse param.
    while((opt = getopt(argc, argv, "hr:f:m:a:l:g:x:s:n:b:")) != EOF)
    {
        switch( opt ) 
        {
            case 'h':
                usage();
                break;
            case 'r':
                receive = true;
                path = optarg;
                break;
            case 'f':
                f_hz = strtod(optarg, &endptr);
                if (optarg == endptr){
                    result = HACKRF_ERROR_INVALID_PARAM;
                    break;
                }
                freq_hz = f_hz;
                automatic_tuning = true;
                break;
            case 'm':
                image_reject = true;
                result = parse_u32(optarg, $image_reject_selection);
                break;
            case 'a':
                amp = true;
                result = parse_u32(optarg, &amp_enable);
                break;
            case 'l':
                result = parse_u32(optarg, &lna_gain);
                break;
            case 'g':
                result = parse_u32(optarg, &vga_gain);
                break;
            case 'x':
                result = parse_u32(optarg, &txvga_gain);
                break;
            case 's':
                f_hz = strtod(optarg, &endptr);
                if (optarg == endptr){
                    result = HACKRF_ERROR_INVALID_PARAM;
                    break;
                }
                sample_rate_hz = f_hz;
                sample_rate = true;
                break;
            case 'n':
                limit_num_samples = true;
                result = parse_u64(optarg, &sample_to_xfer);
                bytes_to_xfer = samples_to_xfer * 2ull;
                break;
            case 'b':
                break;
            default:
                fprintf(stderr, "unknown argument '-%c %s'\n", opt, optarg, hackrf_error_name(result), result);
                usage();
                return EXIT_FAILURE;
        }
    }
	
	hackrf_device_list_free(list);
	hackrf_exit();

	return EXIT_SUCCESS;
}
