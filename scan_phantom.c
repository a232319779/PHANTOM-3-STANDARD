#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "bk5811_demodu.h"

void sigint_callback_handler(int signum);
int rx_callback(hackrf_transfer *transfer);

// my
int parse_opt(int argc, char* argv[], rf_param *rp);
int scan(uint64_t freq_hz);
#define START_FREQ 5725000000
#define RF_PARAM_INIT() {                             \
        .freq_hz = START_FREQ,                 \
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

static hackrf_device* device = NULL;
volatile uint32_t byte_count = 0;
static bool do_exit = false;
static int do_count = 0;
static rf_param rp = RF_PARAM_INIT();
char *buffer = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    int exit_code = EXIT_SUCCESS;

    signal(SIGINT, &sigint_callback_handler); 
    signal(SIGILL, &sigint_callback_handler); 
    signal(SIGFPE, &sigint_callback_handler); 
    signal(SIGSEGV, &sigint_callback_handler); 
    signal(SIGTERM, &sigint_callback_handler); 
    signal(SIGABRT, &sigint_callback_handler); 

    uint64_t freq_hz = rp.freq_hz;
    for(int i = 0; i < 125; i++)
    {
        fprintf(stderr, "channel number : %d\t channel frequency : %llu\n", i, freq_hz);
        scan(freq_hz);
        if ( do_exit)
            break;
        freq_hz +=  FREQ_ONE_MHZ;
    }
    return exit_code;
}

int scan(uint64_t freq_hz)
{
    int result;
    const char* serial_number = NULL;
    if (rp.baseband_filter_bw)
    {
        /* Compute nearest req for bw filter */
        rp.baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw(rp.baseband_filter_bw_hz);
    }

    result = hackrf_init();
    result = hackrf_open_by_serial(serial_number, &device);

    result = hackrf_set_sample_rate_manual(device, rp.sample_rate_hz, 1);
    result = hackrf_set_baseband_filter_bandwidth(device, rp.baseband_filter_bw_hz);
    result = hackrf_set_vga_gain(device, rp.vga_gain);
    result |= hackrf_set_lna_gain(device, rp.lna_gain);
    result |= hackrf_start_rx(device, rx_callback, NULL);

    result = hackrf_set_freq(device, freq_hz);
    result = hackrf_set_amp_enable(device, (uint8_t)rp.amp_enable);

    buffer = (char *)malloc(262144);
    memset(buffer, 0, 262144);

    do_count = 0;
    while( (hackrf_is_streaming(device) == HACKRF_TRUE) && (do_count < 5))
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

    result = hackrf_stop_rx(device); 
    result = hackrf_close(device);
    hackrf_exit();

    return 0;
}

void sigint_callback_handler(int signum)
{
    fprintf(stdout, "Caught signal %d\n", signum);
    do_exit = true;
}

int rx_callback(hackrf_transfer *transfer)
{
    pthread_mutex_lock(&mutex);
    byte_count += transfer->valid_length;
    memcpy(buffer, transfer->buffer, transfer->buffer_length);
    pthread_mutex_unlock(&mutex);
    do_count++;

    return 0;
}
