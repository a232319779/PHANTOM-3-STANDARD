#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include "parse_opt.h"

t_u64toa ascii_u64_data1;
t_u64toa ascii_u64_data2;


static void usage();

void sigint_callback_handler(int signum);
int rx_callback(hackrf_transfer *transfer);

// my
int parse_opt(int argc, char* argv[]);

static transceiver_mode_t transceiver_mode = TRANSCEIVER_MODE_RX;
static hackrf_device* device = NULL;
static FILE *fd = NULL;
volatile uint32_t byte_count = 0;
static bool do_exit = false;
struct timeval time_start;
struct timeval t_start;
extern rf_param rp;

int main(int argc, char *argv[])
{
    int result;
    const char* serial_number = NULL;
    int exit_code = EXIT_SUCCESS;
    struct timeval t_end;
    float time_diff;

    // set default path 
    rp.path = "data/1M_5738_recive_1s.iq";

    if (argc > 1)
    {
        result = parse_opt(argc, argv);
        if (result == EXIT_FAILURE)
            return -1;
    }

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
    CHECK_RESULT(result, "hackrf_init()");

    result = hackrf_open_by_serial(serial_number, &device);
    CHECK_RESULT(result, "hackrf_open()");

    if ( transceiver_mode != TRANSCEIVER_MODE_SS ) {
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
    }

    signal(SIGINT, &sigint_callback_handler); 
    signal(SIGILL, &sigint_callback_handler); 
    signal(SIGFPE, &sigint_callback_handler); 
    signal(SIGSEGV, &sigint_callback_handler); 
    signal(SIGTERM, &sigint_callback_handler); 
    signal(SIGABRT, &sigint_callback_handler); 

    fprintf(stderr, "call hackrf_sample_rate_set(%u Hz/%0.3f MHz)\n", rp.sample_rate_hz, ((float)rp.sample_rate_hz/(float)FREQ_ONE_MHZ));
    result = hackrf_set_sample_rate_manual(device, rp.sample_rate_hz, 1);
    CHECK_RESULT(result, "hackrf_sample_rate_set()");

    fprintf(stderr, "call hackrf_baseband_filter_bandwidth_set(%d Hz/%0.3fMHz)\n",
            rp.baseband_filter_bw_hz, ((float)rp.baseband_filter_bw_hz/(float)FREQ_ONE_MHZ));
    result = hackrf_set_baseband_filter_bandwidth(device, rp.baseband_filter_bw_hz);
    CHECK_RESULT(result, "hackrf_baseband_filter_bandwidth_set()");

    if( transceiver_mode == TRANSCEIVER_MODE_RX ) {
        result = hackrf_set_vga_gain(device, rp.vga_gain);
        result |= hackrf_set_lna_gain(device, rp.lna_gain);
        result |= hackrf_start_rx(device, rx_callback, NULL);
    }
    CHECK_RESULT(result, "hackrf_start_?x()");

    if (rp.automatic_tuning) {
        fprintf(stderr, "call hackrf_set_freq(%s Hz/%0.3f MHz)\n",
                u64toa(rp.freq_hz, &ascii_u64_data1), ((double)rp.freq_hz/(double)FREQ_ONE_MHZ));
        result = hackrf_set_freq(device, rp.freq_hz);
        CHECK_RESULT(result, "hackrf_set_freq()");
    }
    
    if( rp.amp ) {
        fprintf(stderr, "call hackrf_set_amp_enable(%u)\n", rp.amp_enable);
        result = hackrf_set_amp_enable(device, (uint8_t)rp.amp_enable);
        CHECK_RESULT(result, "hackrf_set_amp_enable()");
    }

    if ( rp.limit_num_samples) {
        fprintf(stderr, "samples_to_xfer %s/%sMio\n",
                u64toa(rp.samples_to_xfer, &ascii_u64_data1),
                u64toa((rp.samples_to_xfer/FREQ_ONE_MHZ), &ascii_u64_data2));
    }

    gettimeofday(&t_start, NULL);
    gettimeofday(&time_start, NULL);
    
    fprintf(stderr, "Stop with Ctrl-c\n");
    while( (hackrf_is_streaming(device) == HACKRF_TRUE) &&
            (do_exit == false))
    {
        uint32_t byte_count_now;
        struct timeval time_now;
        float time_difference, rate;
        sleep(1);

        // Linux
        gettimeofday(&time_now, NULL);

        byte_count_now = byte_count;
        byte_count = 0;

        time_difference = TimevalDiff(&time_now, &time_start);
        rate = (float)byte_count_now / time_difference;
        fprintf(stderr, "%4.1f MiB / %5.3f sec = %4.1f MiB/second\n",
                (byte_count_now / 1e6f), time_difference, (rate /1e6f));

        time_start = time_now;

        if ( byte_count_now == 0) {
            exit_code = EXIT_FAILURE;
            fprintf(stderr, "\nCouldn't transfer any bytes for one second.\n");
            break;
        }
    }

    result = hackrf_is_streaming(device);
    if( do_exit )
    {
        fprintf(stderr, "\nUser cancel, exiting...\n");
    }
    else
    {
        fprintf(stderr, "\nExiting... hackrf_is_streaming() result: %s(%d)\n", hackrf_error_name(result), result);
    }

    gettimeofday(&t_end, NULL);
    time_diff = TimevalDiff(&t_end, &t_start);
    fprintf(stderr, "Total time: %5.5f s\n", time_diff);

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
    printf("\t[-h help] # Display this text.\n");
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
    printf("Default set : -f 5738000000 -a 1 -l 32 -g 20 -s 1000000 -n 1000000 -r data/1M_5738_recive_1s.iq\n");
}

int parse_opt(int argc, char *argv[])
{
    char *opt_select = "hf:a:l:g:n:r:s:";
    parse_opt_param(argc, argv, opt_select, usage);
    
    return 0;
}

void sigint_callback_handler(int signum)
{
    fprintf(stdout, "Caught signal %d\n", signum);
    do_exit = true;
}

int rx_callback(hackrf_transfer *transfer)
{
	ssize_t bytes_to_write;
	ssize_t bytes_written;

	if( fd != NULL ) 
	{
		byte_count += transfer->valid_length;
		bytes_to_write = transfer->valid_length;
		if (rp.limit_num_samples) {
			if (bytes_to_write >= rp.bytes_to_xfer) {
				bytes_to_write = rp.bytes_to_xfer;
			}
			rp.bytes_to_xfer -= bytes_to_write;
		}

		bytes_written = fwrite(transfer->buffer, 1, bytes_to_write, fd);
		if ((bytes_written != bytes_to_write)
				|| (rp.limit_num_samples && (rp.bytes_to_xfer == 0))) {
			return -1;
		} else {
			return 0;
		}
	} else {
		return -1;
    } 
}

