#include <stdint.h>

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

#define FREQ_ONE_MHZ (1000000ull)
#define FREQ_MIN_HZ (0ull)  /* 0 Hz */
#define FREQ_MAX_HZ (7250000000ull) /* 7250MHz */
#define DEFAULT_SAMPLE_RATE_HZ (10000000)   /* 10MHz default sample rate */ 

#define U64TOA_MAX_DIGIT (31)
typedef struct _t_u64toa{
    char data[U64TOA_MAX_DIGIT+1];
} t_u64toa;

typedef struct _rf_param{
    uint64_t freq_hz; 
    bool automatic_tuning;
    uint32_t amp_enable;
    bool amp;
    uint32_t sample_rate_hz;
    bool sample_rate;
    bool receive;
    char *path;
    bool limit_num_sample;
}rf_param, *prf_param;
