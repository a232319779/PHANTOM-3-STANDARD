#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "rf_common.h"
#include "bk5811_demodu.h"

void sigint_callback_handler(int signum);
int rx_callback(hackrf_transfer *transfer);
void gen_file_name(char *file_name, char *function_name, uint64_t freq_hz);

// my
int parse_opt(int argc, char* argv[], rf_param *rp);
int scan(uint64_t freq_hz);

#define HACKRF_SAMPLE_NUMBER    262144              // 256k
#define TIMES_PER_CHANNEL       5
#define SIZE_PER_CHANNEL        (HACKRF_SAMPLE_NUMBER * TIMES_PER_CHANNEL)      // 每次处理1M大小的信号

#define IF_ALL  1
#define IF_ONE  0

static hackrf_device* device = NULL;
volatile uint32_t byte_count = 0;
static bool do_exit = false;
static int do_count = 0;
static int do_per_channel = 0;
char *buffer = NULL;
int8_t *channels = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//
FILE *fd = NULL;
static packet_param pp = INIT_PP();

int main(int argc, char *argv[])
{
    int result;
    int exit_code = EXIT_SUCCESS;
    uint8_t i;
    uint32_t channel_number = pp.channel_number;
    rp.freq_hz = pp.start_freq;

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

    if (argc == 2)
        rp.path = argv[1];
   
#if IF_ALL
    char file_name[255] = {0};
    gen_file_name(file_name, "scan_phontom", 0);
    fd = fopen(file_name, "wb");
#endif

    buffer = (char *)malloc(SIZE_PER_CHANNEL);
    channels = (int8_t *)malloc(channel_number);

    for(i = 0; i < channel_number; i++)
    {
        fprintf(stderr, "channel number : %d\t channel frequency : %llu\n", i, freq_hz);
        if(1 == scan(freq_hz))
            channels[i] = 1;
        if ( do_exit)
            break;
        freq_hz +=  FREQ_ONE_MHZ;
    }

    free(channels);
    free(buffer);

#if IF_ALL
    if(fd != NULL){
        fclose(fd);
        fd = NULL;
    }
#endif
    fprintf(stdout, "find signal at these channels:\n");
    int count = 0;
    for(i = 0; i < channel_number; i++)
    {
        if( 1 == channels[i] )
        {
            count++;
            fprintf(stdout, ",%d", i);
        }
    }
    printf("\ntotal %d channels.\n", count);

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

    memset(buffer, 0, SIZE_PER_CHANNEL); 

    do_count = 0;
    do_per_channel = 0;

#if IF_ONE    
    char file_name[255] = {0};
    gen_file_name(file_name, "scan_phantom", );
    fd = fopen(file_name, "wb");
#endif
    while( (hackrf_is_streaming(device) == HACKRF_TRUE) && (do_count < TIMES_PER_CHANNEL));

    pthread_mutex_lock(&mutex);
    long start_position = -1;
    uint8_t l_channel = 0;
    mean(buffer, 0, SIZE_PER_CHANNEL); 
    find_inter(buffer, 0, SIZE_PER_CHANNEL); 
    if( 1 ==  work(buffer, &pp, &start_position, &l_channel))
    {
        isfind = 1;
    }
    memset(buffer, 0, SIZE_PER_CHANNEL);
    memset(g_inter, 0, PACKET_COUNT);
    pthread_mutex_unlock(&mutex);

#if IF_ONE    
    if(fd != NULL){
        fclose(fd);
        fd = NULL;
    }
#endif    

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
    byte_count++; 
    if(do_count < TIMES_PER_CHANNEL)
    {
        pthread_mutex_lock(&mutex);
        memcpy(buffer + do_per_channel, transfer->buffer, transfer->valid_length);
#if IF_ALL||IF_ONE
        fwrite(transfer->buffer, 1, transfer->buffer_length, fd);
#endif
        do_count++;
        //printf("do_per_channel : %d\n", do_per_channel);
        do_per_channel += transfer->buffer_length;
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}

// debug function
void gen_file_name(char *file_name, char *function_name, uint64_t freq_hz)
{
    time_t timep;
    struct tm *p;
    time (&timep);
    p=gmtime(&timep);
    
    sprintf(file_name,"data/%dMHz_%lluMHz_%d-%d-%d-%d-%d_%s.iq", rp.sample_rate_hz/(int)FREQ_ONE_MHZ, freq_hz, p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour+8, p->tm_min, function_name);
}
