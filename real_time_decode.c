#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define FREQ_ONE_HZ             (1000000)
#define START_FREQ              (5725 * FREQ_ONE_HZ)    /* default start frequency 5.725GHz */
#define CHANNEL_NUMBER          (125)                   /* default scan channels 125 */
#define HOPPING_NUMBER          (16)                    /* default hopping number 16 */
#define DEFAULT_SAMPLE_RATE     (4 * FREQ_ONE_HZ)       /* default sample rate 4MHz */
#define DEFAULT_PERIOD          (7)                     /* default period 7ms */
#define DEFAULT_BUFFER_SIZE     (DEFAULT_SAMPLE_RATE * (DEFAULT_PERIOD+1) * 2 / 1000)

static void usage();

static hackrf_device* device = NULL;

int main(int argc, char *argv[])
{
    int result;    
    const char* serial_number = NULL;

    result = hackrf_init();
    CHECK_RESULT(result, "hackrf_init()");

    result = hackrf_open_by_serial(serial_number, &device);
    CHECK_RESULT(result, "hackrf_open()");

    result = hackrf_stop_rx(device);
    CHECK_IF_ELSE(result, "hackrf_stop_rx()");

    result = hackrf_close(device);
    CHECK_IF_ELSE(result, "hackrf_close()");

    hackrf_exit();
    fprintf(stderr, "hackrf_exit() done.\n");

    return 0;
}

static void usage()
{
    printf("help real_time_decode\n");
}
