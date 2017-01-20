#ifndef _COMMON_H_
#define _COMMON_H_
#endif

#include <hackrf.h>

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
