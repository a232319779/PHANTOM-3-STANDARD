#ifndef _RF_COMMON_H
#define _RF_COMMON_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <hackrf.h>

#define FD_BUFFER_SIZE (8*1024)

#define FREQ_ONE_MHZ (1000000ull)
#define FREQ_MIN_HZ (0ull)  /* 0 Hz */
#define FREQ_MAX_HZ (7250000000ull) /* 7250MHz */
#define DEFAULT_SAMPLE_RATE_HZ (1000000)   /* 1MHz default sample rate */ 
#define DEFAULT_FREQ_HZ (5738000000ull) /* 5738MHz */
#define DEFAULT_BASEBAND_FILTER_BANDWIDTH (5000000) /* 5MHz default */

#define BASEBAND_FILTER_BW_MIN (1750000) /* 1.75 MHz min value */
#define BASEBAND_FILTER_BW_MAX (28000000) /* 28 MHz max value */

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

typedef enum {
    TRANSCEIVER_MODE_OFF = 0,
    TRANSCEIVER_MODE_RX = 1,
    TRANSCEIVER_MODE_TX = 2,
    TRANSCEIVER_MODE_SS = 3,
} transceiver_mode_t;

typedef struct _rf_param{
    uint64_t freq_hz; 
    bool automatic_tuning;
    uint32_t amp_enable;
    bool amp;
    uint32_t sample_rate_hz;
    bool sample_rate;
    bool receive;
    char *path;
    uint64_t samples_to_xfer;
    ssize_t bytes_to_xfer;
    bool limit_num_samples;
    uint32_t lna_gain;
    uint32_t vga_gain;
    bool baseband_filter_bw;
    uint32_t baseband_filter_bw_hz;
}rf_param, *prf_param;

/* default param :
 * channel freq : 5738MHz
 * use antenna
 * sample rate : 1MHz
 * use to receive signal
 * sampling in 1 seconds.
 * lna_gain : 32, vga_gain : 20
 * use filter : 1MHz
 * * * * * * * * * * * * * * * *
 * noticed : 'path' should be set when save the signal.
*/
#define RF_PARAM_INIT() { \
        .freq_hz = DEFAULT_FREQ_HZ, \
        .automatic_tuning = true, \
        .amp_enable = 1, \
        .amp = true, \
        .sample_rate_hz = DEFAULT_SAMPLE_RATE_HZ, \
        .sample_rate = true, \
        .receive = true, \
        .path = "data/not_set_path.iq", \
        .samples_to_xfer = DEFAULT_SAMPLE_RATE_HZ, \
        .bytes_to_xfer = 2 * DEFAULT_SAMPLE_RATE_HZ, \
        .limit_num_samples = true, \
        .lna_gain = 32, \
        .vga_gain = 20, \
        .baseband_filter_bw = true, \
        .baseband_filter_bw_hz = 1000000 \
}

// global rf param
//rf_param rp = RF_PARAM_INIT();

#define CHECK_RESULT(result, error_name)    if(result != HACKRF_SUCCESS) {  \
        fprintf(stderr, "%s failed: %s (%d).\n",                                \
                    error_name, hackrf_error_name(result), result);                         \
        usage();                                                                \
        return EXIT_FAILURE;                                                    \
        }    

#define CHECK_IF_ELSE(result, error_name)   if(result != HACKRF_SUCCESS) {  \
        fprintf(stderr, "%s failed: %s (%d)\n",                                 \
                    error_name, hackrf_error_name(result), result);                         \
        }else {                                                                 \
                fprintf(stderr, "%s done.\n", error_name);                              \
        }

#endif
