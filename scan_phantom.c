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

#define HACKRF_SAMPLE_NUMBER    262144
#define TIMES_PER_CHANNEL       10
#define START_FREQ              5725000000
#define CHANNELS_NUMBER         125

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
int8_t channels[CHANNELS_NUMBER] = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    int result;
    int exit_code = EXIT_SUCCESS;
    int8_t i;

    signal(SIGINT, &sigint_callback_handler); 
    signal(SIGILL, &sigint_callback_handler); 
    signal(SIGFPE, &sigint_callback_handler); 
    signal(SIGSEGV, &sigint_callback_handler); 
    signal(SIGTERM, &sigint_callback_handler); 
    signal(SIGABRT, &sigint_callback_handler); 
    
    rp.baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw(rp.baseband_filter_bw_hz);
    
    result = hackrf_init();
    if(result != HACKRF_SUCCESS)
        return -1;
    
    uint64_t freq_hz = rp.freq_hz;
    for(i = 0; i < CHANNELS_NUMBER; i++)
    {
        fprintf(stderr, "channel number : %d\t channel frequency : %llu\n", i, freq_hz);
        if(1 == scan(freq_hz))
            channels[i] = 1;
        if ( do_exit)
            break;
        freq_hz +=  FREQ_ONE_MHZ;
    }

    fprintf(stdout, "find signal at these channels:\n");
    for(i = 0; i < CHANNELS_NUMBER; i++)
    {
        if( 1 == channels[i] )
            fprintf(stdout, ",%d", i);
    }
    fprintf(stdout, "\n\n");

    hackrf_exit();

    return exit_code;
}

int scan(uint64_t freq_hz)
{
    int result;
    int isfind = 0;

    result = hackrf_open(&device);
    if( result != HACKRF_SUCCESS)
    {
        fprintf(stderr, "can not open hackrf.\nexit.");
        exit(0);
    }
    result = hackrf_set_sample_rate_manual(device, rp.sample_rate_hz, 1);
    result = hackrf_set_baseband_filter_bandwidth(device, rp.baseband_filter_bw_hz);
    result = hackrf_set_vga_gain(device, rp.vga_gain);
    result |= hackrf_set_lna_gain(device, rp.lna_gain);
    result |= hackrf_start_rx(device, rx_callback, NULL);
    result = hackrf_set_freq(device, freq_hz);
    result = hackrf_set_amp_enable(device, (uint8_t)rp.amp_enable);

    buffer = (char *)malloc(HACKRF_SAMPLE_NUMBER);
    memset(buffer, 0, HACKRF_SAMPLE_NUMBER);

    do_count = 0;
    while( (hackrf_is_streaming(device) == HACKRF_TRUE) && (do_count < TIMES_PER_CHANNEL))
    { 
        //sleep(1);
        pthread_mutex_lock(&mutex);
        int end_size = 0;
        for(int i = 0; i < HACKRF_SAMPLE_NUMBER; i += PACKET_SIZE)
        {
            end_size = PACKET_SIZE < (HACKRF_SAMPLE_NUMBER - i) ? PACKET_SIZE : (HACKRF_SAMPLE_NUMBER - i - 2);
            mean(buffer, i, end_size);
            find_inter(buffer, i, end_size); 
            if( 1 ==  work(buffer))
                isfind = 1;
            
        }
        memset(buffer, 0, HACKRF_SAMPLE_NUMBER);
        memset(g_inter, 0, PACKET_COUNT);
        pthread_mutex_unlock(&mutex);
    }
    free(buffer);

    result = hackrf_stop_rx(device); 
    result = hackrf_close(device);

    return isfind;
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
