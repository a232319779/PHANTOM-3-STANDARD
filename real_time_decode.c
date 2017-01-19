#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define FREQ_ONE_MHZ             (1000000)
#define START_FREQ              (5725 * FREQ_ONE_MHZ)    /* default start frequency 5.725GHz */
#define CHANNEL_NUMBER          (125)                   /* default scan channels 125 */
#define HOPPING_NUMBER          (16)                    /* default hopping number 16 */
#define DEFAULT_SAMPLE_RATE     (4 * FREQ_ONE_MHZ)       /* default sample rate 4MHz */
#define DEFAULT_PERIOD          (7)                     /* default period 7ms */
#define DEFAULT_BUFFER_SIZE     (DEFAULT_SAMPLE_RATE * (DEFAULT_PERIOD+1) * 2 / 1000)

static void usage();
void sigint_callback_handler(int signum);
void call_signal();
int rx_callback(hackrf_transfer* transfer);
int init_hackrf();
int start_hackrf_rx();
int exit_hackrf();

static hackrf_device* device = NULL;
static int do_exit = false;

int main(int argc, char *argv[])
{
    int result;

    result = init_hackrf();
    call_signal();
    result = start_hackrf_rx();
    for(int i = 0; i < CHANNEL_NUMBER; i++)
    {
        hackrf_set_freq(device, START_FREQ + i * FREQ_ONE_MHZ);
    }
    result = exit_hackrf();

    return 0;
}

static void usage()
{
    printf("help real_time_decode\n");
}

void sigint_callback_handler(int signum)
{
    fprintf(stdout, "Caught signal %d\n", signum);
    do_exit = true;
}

void call_signal()
{
    signal(SIGINT, &sigint_callback_handler); 
    signal(SIGILL, &sigint_callback_handler); 
    signal(SIGFPE, &sigint_callback_handler); 
    signal(SIGSEGV, &sigint_callback_handler); 
    signal(SIGTERM, &sigint_callback_handler); 
    signal(SIGABRT, &sigint_callback_handler); 
}

int rx_callback(hackrf_transfer *transfer)
{
    return 0; 
}

int init_hackrf()
{
    int result;    
    const char* serial_number = NULL;

    result = hackrf_init();
    CHECK_RESULT(result, "hackrf_init()");

    result = hackrf_open_by_serial(serial_number, &device);
    CHECK_RESULT(result, "hackrf_open()");

    return result;
}

int start_hackrf_rx()
{
    int result;
    int lna_gain = 8;
    int vga_gain = 20;

    result = hackrf_set_sample_rate_manual(device, DEFAULT_SAMPLE_RATE, 1);
    CHECK_RESULT(result, "hackrf_smaple_rate_set()");

    result = hackrf_set_vga_gain(device, vga_gain);
    result |= hackrf_set_lna_gain(device, lna_gain);
    result |= hackrf_start_rx(device, rx_callback, NULL);
    CHECK_RESULT(result, "hackrf_start_?x()");

    return result;
}

int exit_hackrf()
{
    int result;
    result = hackrf_stop_rx(device);
    CHECK_IF_ELSE(result, "hackrf_stop_rx()");

    result = hackrf_close(device);
    CHECK_IF_ELSE(result, "hackrf_close()");

    hackrf_exit();
    fprintf(stderr, "hackrf_exit() done.\n");

    return result;
}
