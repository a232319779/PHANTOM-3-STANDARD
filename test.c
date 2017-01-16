#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <hackrf.h>

#include "test.h"

#ifndef strtoull 
#define strtoull _strtoui64
#endif

#ifndef snprintf
#define snprintf _snprintf
#endif

t_u64toa ascii_u64_data1;
t_u64toa ascii_u64_data2;

void usage();
char* u64toa(uint64_t val, t_u64toa* str);
int parse_u64(char* s, uint64_t* const value);
static char *stringrev(char *str); 

int main(int argc, char *argv[])
{
    usage();
    return 0;
}

void usage()
{
    printf("Usage:\n");
    printf("\t[-f freq_hz] # Frequency in Hz [%sMHz to %sMHz].\n",
            u64toa((FREQ_MIN_HZ/FREQ_ONE_MHZ), &ascii_u64_data1),
            u64toa((FREQ_MAX_HZ/FREQ_ONE_MHZ), &ascii_u64_data2));
    printf("\t[-a amp_enable] # RX/TX RF amplifier 1=Enable, 0=Disable.\n");
    printf("\t[-l gain_db] # RX LNA (IF) gain, 0-40dB, 8dB steps.\n");
    printf("\t[-g gain_db] # RX VGA (baseband) gain, 0-62dB, 2dB steps.\n");
    printf("\t[-s sample_rate_hz] # Sample rate in Hz (4/8/10/12.5/16/20MHz, default %sMHz).\n",
           u64toa((DEFAULT_SAMPLE_RATE_HZ/FREQ_ONE_MHZ),&ascii_u64_data1));
    printf("\t[-n num_samples] # Number of samples to transfer (default is unlimited).\n");
    printf("\t[-r <filename>] # Receive data into file (use '-' for stdout).\n");
}

char* u64toa(uint64_t val, t_u64toa* str)
{
	#define BASE (10ull) /* Base10 by default */
	uint64_t sum;
	int pos;
	int digit;
	int max_len;
	char* res;

	sum = val;
	max_len = U64TOA_MAX_DIGIT;
	pos = 0;

	do
	{
		digit = (sum % BASE);
		str->data[pos] = digit + '0';
		pos++;

		sum /= BASE;
	}while( (sum>0) && (pos < max_len) );

	if( (pos == max_len) && (sum>0) )
		return NULL;

	str->data[pos] = '\0';
	res = stringrev(str->data);

	return res;
}

static char *stringrev(char *str)
{
	char *p1, *p2;

	if(! str || ! *str)
		return str;

	for(p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
	return str;
}

int parse_u64(char* s, uint64_t* const value) 
{
	uint_fast8_t base = 10;
	char* s_end;
	uint64_t u64_value;

	if( strlen(s) > 2 ) {
		if( s[0] == '0' ) {
			if( (s[1] == 'x') || (s[1] == 'X') ) {
				base = 16;
				s += 2;
			} else if( (s[1] == 'b') || (s[1] == 'B') ) {
				base = 2;
				s += 2;
			}
		}
	}

	s_end = s;
	u64_value = strtoull(s, &s_end, base);
	if( (s != s_end) && (*s_end == 0) ) {
		*value = u64_value;
		return HACKRF_SUCCESS;
	} else {
		return HACKRF_ERROR_INVALID_PARAM;
	}
}
