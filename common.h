#ifndef _COMMON_H_
#define _COMMON_H_
#endif

#include <hackrf.h>

#ifndef bool
typedef int bool;
#define true 1
#define false 0
#endif

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
