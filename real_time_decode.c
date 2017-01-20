#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "test.h"
#include "bk5811_demodu.h"

t_u64toa ascii_u64_data1;
t_u64toa ascii_u64_data2;

static void usage();
char* u64toa(uint64_t val, t_u64toa* str);
int parse_u32(char* s, uint32_t* const value); 
int parse_u64(char* s, uint64_t* const value);
static char *stringrev(char *str);

void sigint_callback_handler(int signum);
int rx_callback(hackrf_transfer *transfer);

// my
int parse_opt(int argc, char* argv[], rf_param *rp);

#define RF_PARAM_INIT() {                             \
        .freq_hz = DEFAULT_FREQ_HZ,                 \
        .automatic_tuning = true,                  \
        .amp_enable = 1,                            \
        .amp = true,                               \
        .sample_rate_hz = DEFAULT_SAMPLE_RATE_HZ,   \
        .sample_rate = true,                       \
        .receive = true,                           \
        .path = "data.iq",                               \
        .samples_to_xfer = 0,                       \
        .bytes_to_xfer = 0,                         \
        .limit_num_samples = false,                 \
        .lna_gain = 40,                              \
        .vga_gain = 20,                             \
        .baseband_filter_bw = true,                \
        .baseband_filter_bw_hz = 1000000                  \
}

static transceiver_mode_t transceiver_mode = TRANSCEIVER_MODE_RX;
static hackrf_device* device = NULL;
static FILE *fd = NULL;
volatile uint32_t byte_count = 0;
static bool do_exit = false;
struct timeval time_start;
struct timeval t_start;
static rf_param rp = RF_PARAM_INIT();
char *buffer = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    int result;
    const char* serial_number = NULL;
    int exit_code = EXIT_SUCCESS;

    result = parse_opt(argc, argv, &rp);
    if (result == EXIT_FAILURE)
        return -1;

    if (rp.baseband_filter_bw)
    {
        /* Compute nearest req for bw filter */
        rp.baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw(rp.baseband_filter_bw_hz);
    }

    if( rp.receive){
        transceiver_mode = TRANSCEIVER_MODE_RX;
    }

    result = hackrf_init();
    CHECK_RESULT(result, "hackrf_init()");

    result = hackrf_open_by_serial(serial_number, &device);
    CHECK_RESULT(result, "hackrf_open()");

    if( transceiver_mode == TRANSCEIVER_MODE_RX )
    {
        if ( strcmp(rp.path, "-") == 0)
            fd = stdout;
        else
            fd = fopen(rp.path, "wb");
    }

    if ( fd == NULL ) {
        fprintf(stderr, "Failed to open file: %s\n", rp.path);
        return EXIT_FAILURE;
    }

    result = setvbuf(fd, NULL, _IOFBF, FD_BUFFER_SIZE);
    if ( result != 0) {
        fprintf(stderr, "setvbuf() failed: %d\n", result);
        usage();
        return EXIT_FAILURE;
    }
   

    signal(SIGINT, &sigint_callback_handler); 
    signal(SIGILL, &sigint_callback_handler); 
    signal(SIGFPE, &sigint_callback_handler); 
    signal(SIGSEGV, &sigint_callback_handler); 
    signal(SIGTERM, &sigint_callback_handler); 
    signal(SIGABRT, &sigint_callback_handler); 

    result = hackrf_set_sample_rate_manual(device, rp.sample_rate_hz, 1);
    CHECK_RESULT(result, "hackrf_sample_rate_set()");

    result = hackrf_set_baseband_filter_bandwidth(device, rp.baseband_filter_bw_hz);
    CHECK_RESULT(result, "hackrf_baseband_filter_bandwidth_set()");

    result = hackrf_set_vga_gain(device, rp.vga_gain);
    result |= hackrf_set_lna_gain(device, rp.lna_gain);
    result |= hackrf_start_rx(device, rx_callback, NULL);
    CHECK_RESULT(result, "hackrf_start_?x()");

    result = hackrf_set_freq(device, rp.freq_hz);
    CHECK_RESULT(result, "hackrf_set_freq()");

    result = hackrf_set_amp_enable(device, (uint8_t)rp.amp_enable);
    CHECK_RESULT(result, "hackrf_set_amp_enable()");

    buffer = (char *)malloc(262144);
    memset(buffer, 0, 262144);

    while( (hackrf_is_streaming(device) == HACKRF_TRUE) &&
            (do_exit == false))
    { 
        //sleep(1);
        pthread_mutex_lock(&mutex);
        int end_size = 0;
        for(int i = 0; i < 262144; i += PACKET_SIZE)
        {
            end_size = PACKET_SIZE < (262144 - i) ? PACKET_SIZE : (262144 - i - 2);
            mean(buffer, i, end_size);
            find_inter(buffer, i, end_size); 
            work(buffer);
        }
        memset(buffer, 0, 262144);
        memset(g_inter, 0, 1000);
        pthread_mutex_unlock(&mutex);
    }
    free(buffer);

    result = hackrf_is_streaming(device);
    if( do_exit )
    {
        fprintf(stderr, "\nUser cancel, exiting...\n");
    }
    else
    {
        fprintf(stderr, "\nExiting... hackrf_is_streaming() result: %s(%d)\n", hackrf_error_name(result), result);
    }

    if( device != NULL)
    {
        if (rp.receive)
        {
            result = hackrf_stop_rx(device); 
            CHECK_IF_ELSE(result, "hackrf_stop_rx()");
        }

        result = hackrf_close(device);
        CHECK_IF_ELSE(result, "hackrf_close()");

        hackrf_exit();
        fprintf(stderr, "hackrf_exit() done.\n");
    }

    fprintf(stderr, "exit\n");

    return exit_code;
}

static void usage()
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

void sigint_callback_handler(int signum)
{
    fprintf(stdout, "Caught signal %d\n", signum);
    do_exit = true;
}

int rx_callback(hackrf_transfer *transfer)
{
	ssize_t bytes_to_write;
	//ssize_t bytes_written;

    pthread_mutex_lock(&mutex);
	if( fd != NULL ) 
	{
		byte_count += transfer->valid_length;
		bytes_to_write = transfer->valid_length;
		//bytes_written = fwrite(transfer->buffer, 1, bytes_to_write, fd);
        //memcpy(buffer, transfer->buffer, transfer->buffer_length);
        for(int i = 0; i < transfer->valid_length; i++)
            buffer[i] = transfer->buffer[i];
    } 
    pthread_mutex_unlock(&mutex);

    return 0;
}
