#include "parse_opt.h"

rf_param rp = RF_PARAM_INIT();
packet_param pp = INIT_PP();

// my
int parse_opt_param(int argc, char *argv[], char *opt_select, void (*helpinfo)())
{
    int opt;
    int result;
    char* endptr;
    double f_hz;
    
    while( (opt = getopt(argc, argv, opt_select)) != EOF)
    {
       result = HACKRF_SUCCESS;
       switch( opt )
       {
           case 'h' :
               helpinfo();
               exit(0);
           case 'i' :
               pp.preamble_len = (uint8_t)atoi(optarg);
               break;
           case 'j' :
               result = parse_u64(optarg, &pp.dest_preamble);
               break;
           case 'm' :
               pp.address_len = (uint8_t)atoi(optarg);
               break;
           case 'e' :
               pp.is_use_pcf = (uint8_t)atoi(optarg);
               break;
           case 'p' :
               pp.pcf_len = (uint8_t)atoi(optarg);
               break;
           case 'c' :
               pp.crc_len = (uint8_t)atoi(optarg);
               break;
           case 't' :
               result = parse_u32(optarg, &pp.slot_number);
               break;
           case 'q' :
               result = parse_u32(optarg, &pp.channel_number);
               break;
           case 'S' :
               pp.size_per_channel = (uint8_t)atoi(optarg);
               break;
           case 'y' :
               result = parse_u32(optarg, &pp.period);
               break;
           case 'f' :
               f_hz = strtod(optarg, &endptr);
               if (optarg == endptr) {
                   result = HACKRF_ERROR_INVALID_PARAM;
                   break;
               }
               rp.freq_hz = f_hz;
               rp.automatic_tuning = true;
               pp.start_freq = f_hz;
               break;
           case 'a' :
               rp.amp = true;
               result = parse_u32(optarg, &rp.amp_enable);
               break;
           case 'l' :
               result = parse_u32(optarg, &rp.lna_gain);
               break;
           case 'g' :
               result = parse_u32(optarg, &rp.vga_gain);
               break;
           case 's' :
               f_hz = strtod(optarg, &endptr);
               if (optarg == endptr) {
                   result = HACKRF_ERROR_INVALID_PARAM;
                   break;
               }
               rp.sample_rate_hz = f_hz;
               rp.sample_rate = true;
               pp.sample_rate = f_hz;
               pp.sampler_per_symbol = f_hz / FREQ_ONE_MHZ;
               break;
           case 'n' :
               rp.limit_num_samples = true;
               result = parse_u64(optarg, &rp.samples_to_xfer);
               rp.bytes_to_xfer = rp.samples_to_xfer * 2ull;
               break;
           case 'r' :
               rp.receive = true;
               rp.path = optarg;
               break;
           case 'b' :
               f_hz = strtod(optarg, &endptr);
               if (optarg == endptr) {
                   result = HACKRF_ERROR_INVALID_PARAM;
                   break;
               }
               rp.baseband_filter_bw_hz = f_hz;
               rp.baseband_filter_bw = true;
               break;
           default :
               fprintf(stderr, "unknown argument '-%c %s'", opt, optarg);
               helpinfo();
               exit(0);
       }
    }
    return result;
}

int parse_u32(char* s, uint32_t* const value)
{
	uint_fast8_t base = 10;
	char* s_end;
	uint64_t ulong_value;

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
	ulong_value = strtoul(s, &s_end, base);
	if( (s != s_end) && (*s_end == 0) ) {
		*value = (uint32_t)ulong_value;
		return 0;
	} else {
		return -2;
	}
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
		return 0;
	} else {
		return -2;
	}
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

char *stringrev(char *str)
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

float TimevalDiff(const struct timeval *a, const struct timeval *b)
{
   return (a->tv_sec - b->tv_sec) + 1e-6f * (a->tv_usec - b->tv_usec);
}

