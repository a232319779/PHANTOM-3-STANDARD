#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "bk5811_demodu.h"
#include "parse_opt.h"

void sigint_callback_handler(int signum);
int rx_callback(hackrf_transfer *transfer);
void gen_file_name(char *file_name, char *function_name, uint64_t freq_hz);

// my
static void usage();
int parse_opt(int argc, char* argv[]);
int scan(uint64_t freq_hz);

// 每个信道采样1M大小
#define HACKRF_SAMPLE_NUMBER    262144              // 256k
#define TIMES_PER_CHANNEL       4
#define SIZE_PER_CHANNEL        (HACKRF_SAMPLE_NUMBER * TIMES_PER_CHANNEL)      // 每次处理1M大小的信号

#define IF_ALL  0
#define IF_ONE  0

static hackrf_device* device = NULL;
static bool do_exit = false;

static int do_count = 0;
static int do_per_channel = 0;
static uint64_t buffer_length;
static uint8_t loop_times;

// 信道号不大于200
int8_t channels[200] = {0};
float g_period = 99999999.9;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//
FILE *fd = NULL;
char *buffer = NULL;
decode_param *dp = NULL;
s_packet *sp = NULL;
extern packet_param pp;
extern rf_param rp;

int main(int argc, char *argv[])
{
    int result;
    int exit_code = EXIT_SUCCESS;
    uint8_t i;
    uint32_t channel_number;
    uint64_t freq_hz;

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
    

    if (argc > 1)
        parse_opt(argc, argv);
   
    channel_number = pp.channel_number;
    freq_hz = pp.start_freq;
    buffer_length = (SIZE_PER_CHANNEL * pp.size_per_channel);
    loop_times = (TIMES_PER_CHANNEL * pp.size_per_channel);

#if IF_ALL
    char file_name[255] = {0};
    gen_file_name(file_name, "scan_phontom", 0);
    rp.path = file_name;
    fd = fopen(rp.path, "wb");
#endif

    buffer = (char *)malloc(buffer_length);
    dp = (decode_param *)malloc(sizeof(decode_param));
    sp = (s_packet *)malloc(sizeof(s_packet));

    for(i = 0; i < channel_number; i++)
    {
        fprintf(stderr, "channel number : %d\t channel frequency : %llu\n", i, freq_hz);
        scan(freq_hz);
        if ( do_exit)
            break;
        freq_hz +=  FREQ_ONE_MHZ;
    }

#if IF_ALL
    if(fd != NULL){
        fclose(fd);
        fd = NULL;
    }
#endif
    fprintf(stdout, "find signal at these channels:\n");
    int count = 0;
    for(i = 0; i < 200; i++)
    {
        if( channels[i] )
        {
            count++;
            fprintf(stdout, ",%d", i);
        }
    }
    printf("\ntotal_slots : %d.\n", count);
    printf("period : %fms.\n", g_period);
    printf("::) signal per period : period / total_slots = %fms\n", g_period / count);

    free(dp);
    free(sp);
    free(buffer);

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

    memset(buffer, 0, buffer_length); 
    memset(dp, 0, sizeof(decode_param));

    do_count = 0;
    do_per_channel = 0;

#if IF_ONE    
    char file_name[255] = {0};
    gen_file_name(file_name, "scan_phantom", );
    rp.path = file_name;
    fd = fopen(rp.path, "wb");
#endif
    while( (hackrf_is_streaming(device) == HACKRF_TRUE) && (do_count < loop_times));

    pthread_mutex_lock(&mutex);

    long start_p = 0;
    float diff_time = 0.0;
    float period = 9999999.9;

    mean(buffer, 0, buffer_length, dp); 
    find_inter(buffer, 0, buffer_length, dp); 
    while(dp->current < dp->total)
    {
        memset(sp, 0, sizeof(s_packet));

        if(1 == work(buffer, dp, &pp, sp) )
        {
            printf("channel : %d,\tpreamble : %llX,\taddress : %05llX,\tpayload length : %d,\tpid : %d,\tno_ack : %d,\t", sp->channel, sp->preamble, sp->address, sp->payload_len, sp->pid, sp->no_ack);
            printf("payload : ");
            for(int j = 0; j < sp->payload_len; j++)
                printf("%02X", sp->packet_buffer[j]);
            printf(",\tcrc : %llX\n", sp->crc);
            isfind = 1;

            if(start_p)
            {
                    diff_time = calc_diff_time(start_p, sp->start_position, pp.sample_rate);
                    if(diff_time < period)
                        period = diff_time;
            }
            start_p = sp->start_position;
            channels[sp->channel]++;
        }
        dp->current += 2;
    }

    if(start_p)
        printf("! : %fms may be the period.\n", period);
    if(period < g_period)
        g_period = period;
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
    pthread_mutex_lock(&mutex);
    memcpy(buffer + do_per_channel, transfer->buffer, transfer->valid_length);
#if IF_ALL||IF_ONE
    fwrite(transfer->buffer, 1, transfer->valid_length, fd);
#endif
    do_per_channel += transfer->valid_length;
    do_count++;
    pthread_mutex_unlock(&mutex);

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

static void usage()
{
    printf("Usage:\n");
    printf("\t[-h] # Display this text.\n");
    printf("\t[-i] # Preamble length [1 to 8].Default 1.\n");
    printf("\t[-j] # Preamble [1 to 8 bytes].Default \'0xAA\'.\n");
    printf("\t[-m] # Mac address [1 to 5].Default 5.\n");
    printf("\t[-e] # If use esb [1 yes, 0 no].Default 1.\n");
    printf("\t[-p] # Pcf len. Default 2.\n");
    printf("\t[-c] # Crc len. Default 2.\n");
    printf("\t[-q] # Channles count, should less 200. Default 127.\n");
    printf("\t[-S] # Size per channel. Default 1.\n");
    printf("\t[-f] # Start frequency in Hz [1MHz to 6000MHz].\n");
    printf("\t[-a] # RX/TX RF amplifier 1=Enable, 0=Disable.\n");
    printf("\t[-l] # RX LNA (IF) gain, 0-40dB, 8dB steps.\n");
    printf("\t[-g] # RX VGA (baseband) gain, 0-62dB, 2dB steps.\n");
    printf("\t[-s] # Sample rate in Hz (4/8/10/12.5/16/20MHz, default 1MHz).\n");
    printf("Default set : -i 1 -j 0xAA -m 5 -e 1 -p 2 -c 2 -q 127 -S 1 -f 5725000000 -a 1 -l 32 -g 20 -s 1000000\n");
}


int parse_opt(int argc, char *argv[])
{
    
    char *opt_select = "hi:j:m:e:p:c:q:f:a:l:g:s:S:";
    parse_opt_param(argc, argv, opt_select, usage);

    return 0;
}   
