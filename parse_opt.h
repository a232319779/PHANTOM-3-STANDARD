#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "rf_common.h"
#include "packet_common.h"

#define U64TOA_MAX_DIGIT (31)
typedef struct _t_u64toa{
    char data[U64TOA_MAX_DIGIT+1];
} t_u64toa;

extern rf_param rp;
extern packet_param pp;

int parse_u32(char* s, uint32_t* const value); 
int parse_u64(char* s, uint64_t* const value); 
char* u64toa(uint64_t val, t_u64toa* str);
char *stringrev(char *str);
float TimevalDiff(const struct timeval *a, const struct timeval *b);

int parse_opt_param(int argc, char *argv[], char* opt_select, void (*help_info)());
