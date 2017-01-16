#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <hackrf.h>

#include "test.h"

t_u64toa ascii_u64_data1;
t_u64toa ascii_u64_data2;

void usage();
char* u64toa(uint64_t val, t_u64toa* str);
int parse_u32(char* s, uint32_t* const value); 
int parse_u64(char* s, uint64_t* const value);
static char *stringrev(char *str); 


// my
int parse_opt(int argc, char* argv[], rf_param *rp);

#define RF_PARAM_INIT {                             \
        .freq_hz = DEFAULT_FREQ_HZ,                 \
        .automatic_tuning = false,                  \
        .amp_enable = 0,                            \
        .amp = false,                               \
        .sample_rate_hz = DEFAULT_SAMPLE_RATE_HZ,   \
        .sample_rate = false,                       \
        .receive = false,                           \
        .path = NULL,                               \
        .samples_to_xfer = 0,                       \
        .bytes_to_xfer = 0,                         \
        .limit_num_samples = false,                 \
        .lna_gain = 8,                              \
        .vga_gain = 20,                             \
        .baseband_filter_bw = false,                \
        .baseband_filter_bw_hz = 0                  \
}

static transceiver_mode_t transceiver_mode = TRANSCEIVER_MODE_RX;

int main(int argc, char *argv[])
{
    int result;
    rf_param rp = RF_PARAM_INIT;

    result = parse_opt(argc, argv, &rp);
    if (result == EXIT_FAILURE)
        return -1;

    if (rp.baseband_filter_bw)
    {
        /* Compute nearest req for bw filter */
        rp.baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw(rp.baseband_filter_bw_hz);
    }
    else
    {
        /* Compute default value depending on sample rate */
        rp.baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw_round_down_lt(rp.sample_rate_hz);
    }

    if( rp.receive){
        transceiver_mode = TRANSCEIVER_MODE_RX;
    }

    result = hackrf_init();
    if( result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_init() failed: %s (%d).\n", hackrf_error_name(result), result);
        usage();
        return EXIT_FAILURE;
    }

    return 0;
}

void usage()
{
    printf("Usage:\n");
    printf("\t[-f freq_hz] # Frequency in Hz [%sMHz to %sMHz].\n",
            u64toa((FREQ_MIN_HZ/FREQ_ONE_MHZ), &ascii_u64_data1),
            u64toa((FREQ_MAX_HZ/FREQ_ONE_MHZ), &ascii_u64_data2));
    printf("\t[-a amp_enable] # RX/TX RF amplifier 1=Enable, 0=Disable.\n");
    printf("\t[-l gain_db] # RX LNA (IF) gain, 0-40dB, 8dB steps.\n");
    printf("\t[-g gain_db] # RX VGA (baseband) gain, 0-62dB, 2dB steps.\n");
    printf("\t[-s sample_rate_hz] # Sample rate in Hz (4/8/10/12.5/16/20MHz, default %sMHz).\n",
           u64toa((DEFAULT_SAMPLE_RATE_HZ/FREQ_ONE_MHZ),&ascii_u64_data1));
    printf("\t[-n num_samples] # Number of samples to transfer (default is unlimited).\n");
    printf("\t[-r <filename>] # Receive data into file (use '-' for stdout).\n");
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

int parse_u32(char* s, uint32_t* const value) {
	uint_fast8_t base = 10;
	char* s_end;
	uint64_t ulong_value;

	if( strlen(s) > 2 ) {
		if( s[0] == '0' ) {
			if( (s[1] == 'x') || (s[1] == 'X') ) {
				base = 16;
				s += 2;
			} else if( (s[1] == 'b') || (s[1] == 'B') ) {
				base = 2;
				s += 2;
			}
		}
	}

	s_end = s;
	ulong_value = strtoul(s, &s_end, base);
	if( (s != s_end) && (*s_end == 0) ) {
		*value = (uint32_t)ulong_value;
		return HACKRF_SUCCESS;
	} else {
		return HACKRF_ERROR_INVALID_PARAM;
	}
}

int parse_u64(char* s, uint64_t* const value) 
{
	uint_fast8_t base = 10;
	char* s_end;
	uint64_t u64_value;

	if( strlen(s) > 2 ) {
		if( s[0] == '0' ) {
			if( (s[1] == 'x') || (s[1] == 'X') ) {
				base = 16;
				s += 2;
			} else if( (s[1] == 'b') || (s[1] == 'B') ) {
				base = 2;
				s += 2;
			}
		}
	}

	s_end = s;
	u64_value = strtoull(s, &s_end, base);
	if( (s != s_end) && (*s_end == 0) ) {
		*value = u64_value;
		return HACKRF_SUCCESS;
	} else {
		return HACKRF_ERROR_INVALID_PARAM;
	}
}

// my
int parse_opt(int argc, char *argv[], rf_param *rp)
{
    int opt;
    int result;
    char* endptr;
    double f_hz;
    
    while( (opt = getopt(argc, argv, "hf:a:l:g:s:n:r:b:")) != EOF)
    {
       result = HACKRF_SUCCESS;
       switch( opt )
       {
           case 'h' :
               usage();
               break;
           case 'f' :
               f_hz = strtod(optarg, &endptr);
               if (optarg == endptr) {
                   result = HACKRF_ERROR_INVALID_PARAM;
                   break;
               }
               rp->freq_hz = f_hz;
               rp->automatic_tuning = true;
               break;
           case 'a' :
               rp->amp = true;
               result = parse_u32(optarg, &rp->amp_enable);
               break;
           case 'l' :
               result = parse_u32(optarg, &rp->lna_gain);
               break;
           case 'g' :
               result = parse_u32(optarg, &rp->vga_gain);
               break;
           case 's' :
               f_hz = strtod(optarg, &endptr);
               if (optarg == endptr) {
                   result = HACKRF_ERROR_INVALID_PARAM;
                   break;
               }
               rp->sample_rate_hz = f_hz;
               rp->sample_rate = true;
               break;
           case 'n' :
               rp->limit_num_samples = true;
               result = parse_u64(optarg, &rp->samples_to_xfer);
               rp->bytes_to_xfer = rp->samples_to_xfer * 2ull;
               break;
           case 'r' :
               rp->receive = true;
               rp->path = optarg;
               break;
           case 'b' :
               f_hz = strtod(optarg, &endptr);
               if (optarg == endptr) {
                   result = HACKRF_ERROR_INVALID_PARAM;
                   break;
               }
               rp->baseband_filter_bw_hz = f_hz;
               rp->baseband_filter_bw = true;
               break;
           default :
               fprintf(stderr, "unknown argument '-%c %s'", opt, optarg);
               usage();
               return EXIT_FAILURE;
       }
    }
    return result;
}
