#include <stdint.h>
#include <sys/types.h>
#include "common.h"

#define FD_BUFFER_SIZE (8*1024)

#define FREQ_ONE_MHZ (1000000ull)
#define FREQ_MIN_HZ (0ull)  /* 0 Hz */
#define FREQ_MAX_HZ (7250000000ull) /* 7250MHz */
#define DEFAULT_SAMPLE_RATE_HZ (4000000)   /* 10MHz default sample rate */ 
#define DEFAULT_FREQ_HZ (5743000000ull) /* 900MHz */
#define DEFAULT_BASEBAND_FILTER_BANDWIDTH (5000000) /* 5MHz default */

#define BASEBAND_FILTER_BW_MIN (1750000) /* 1.75 MHz min value */
#define BASEBAND_FILTER_BW_MAX (28000000) /* 28 MHz max value */

#define U64TOA_MAX_DIGIT (31)
typedef struct _t_u64toa{
    char data[U64TOA_MAX_DIGIT+1];
} t_u64toa;

