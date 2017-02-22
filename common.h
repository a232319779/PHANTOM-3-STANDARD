#ifndef _COMMON_H_
#define _COMMON_H_
#endif

#include <hackrf.h>
#include <stdint.h>
#include <sys/types.h>

#define FD_BUFFER_SIZE (8*1024)

#define FREQ_ONE_MHZ (1000000ull)
#define FREQ_MIN_HZ (0ull)  /* 0 Hz */
#define FREQ_MAX_HZ (7250000000ull) /* 7250MHz */
#define DEFAULT_SAMPLE_RATE_HZ (1000000)   /* 1MHz default sample rate */ 
#define DEFAULT_FREQ_HZ (5743000000ull) /* 5743MHz */
#define DEFAULT_BASEBAND_FILTER_BANDWIDTH (5000000) /* 5MHz default */

#define BASEBAND_FILTER_BW_MIN (1750000) /* 1.75 MHz min value */
#define BASEBAND_FILTER_BW_MAX (28000000) /* 28 MHz max value */

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#define U64TOA_MAX_DIGIT (31)
typedef struct _t_u64toa{
    char data[U64TOA_MAX_DIGIT+1];
} t_u64toa;

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
