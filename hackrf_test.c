#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "common.h"

void sigint_callback_handler(int signum);
int rx_callback(hackrf_transfer *transfer);
int scan_signal_channel(uint64_t freq_hz);

#define START_FREQ              5725000000
#define TOTAL_NUMBER            125

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
static bool do_exit = false;
static rf_param rp = RF_PARAM_INIT();

int main(int argc, char *argv[])
{
    int exit_code = 0;
    int result;
    uint64_t i;

    signal(SIGINT, &sigint_callback_handler); 
    signal(SIGILL, &sigint_callback_handler); 
    signal(SIGFPE, &sigint_callback_handler); 
    signal(SIGSEGV, &sigint_callback_handler); 
    signal(SIGTERM, &sigint_callback_handler); 
    signal(SIGABRT, &sigint_callback_handler); 
    
    rp.baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw(rp.baseband_filter_bw_hz);
    
    uint64_t freq_hz = rp.freq_hz;
    result = hackrf_init();

    for(uint64_t j = 0; j < 1000; j++)
    {
        int a = clock();
        for(i = 0; i < j; i++)
        {
            //scan_signal_channel(freq_hz);
            //result = hackrf_init();
            result = hackrf_open_by_serial(NULL, &device);
            result = hackrf_set_sample_rate_manual(device, rp.sample_rate_hz, 1);
            result = hackrf_set_baseband_filter_bandwidth(device, rp.baseband_filter_bw_hz);
            result = hackrf_set_vga_gain(device, rp.vga_gain);
            result |= hackrf_set_lna_gain(device, rp.lna_gain);
            result |= hackrf_start_rx(device, rx_callback, NULL);
            while( (hackrf_is_streaming(device) == HACKRF_TRUE) )break; 
            result = hackrf_set_freq(device, freq_hz);
            result = hackrf_set_amp_enable(device, (uint8_t)rp.amp_enable);
            result = hackrf_stop_rx(device); 
            result = hackrf_close(device);
            //hackrf_exit();
        }
        int b = clock();
        float dlt = (b - a) * 1000.0 / (j * CLOCKS_PER_SEC);
        printf("times : %llu\tdlt : %fms\n", j, dlt);
        if(do_exit)break;
    }
    hackrf_exit();

    return exit_code;
}

int scan_signal_channel(uint64_t freq_hz)
{
    int result;

    result = hackrf_open(&device);
    result = hackrf_set_sample_rate_manual(device, rp.sample_rate_hz, 1);
    result = hackrf_set_baseband_filter_bandwidth(device, rp.baseband_filter_bw_hz);
    //result = hackrf_set_vga_gain(device, rp.vga_gain);
    //result |= hackrf_set_lna_gain(device, rp.lna_gain);
    result |= hackrf_start_rx(device, rx_callback, NULL);
    result = hackrf_set_freq(device, freq_hz);
    //result = hackrf_set_amp_enable(device, (uint8_t)rp.amp_enable);
    result = hackrf_stop_rx(device); 
    result = hackrf_close(device);

    return 0;
}

void sigint_callback_handler(int signum)
{
    fprintf(stdout, "Caught signal %d\n", signum);
    do_exit = true;
}

int rx_callback(hackrf_transfer *transfer)
{
    //pthread_mutex_lock(&mutex);
    //pthread_mutex_unlock(&mutex);
    return 0;
}
