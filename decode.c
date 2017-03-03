#include <memory.h>
#include <getopt.h>
#include "bk5811_demodu.h"

static packet_param pp = INIT_PP();
static void usage();
int parse_u32(char* s, uint32_t* const value); 
int parse_u64(char* s, uint64_t* const value);
int parse_opt(int argc, char *argv[], packet_param *pp);

char *sig_file = "data/1M_5738_recive_1s.iq";


int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if(-2 == parse_opt(argc, argv, &pp))
            return -1;
    }
    char *buffer = NULL;
    long file_length = 0;
    long start_position = -1;
    uint8_t channel = 0;
    long size_per_period, end = 0;

    get_signal_data(sig_file, &buffer, &file_length);

    size_per_period = (pp.sample_rate * pp.slot_number * pp.period * 2 / 1000);
    
    for(int i = 0; i < file_length; i += size_per_period)
    {
        end = file_length - i;
        end = end < size_per_period ? end : size_per_period;
        memset(g_inter, 0, sizeof(g_inter));
        mean(buffer, i, end);
        find_inter(buffer, i, end);
        //set_inter(file_length);
        work(buffer, &start_position, &channel, &pp);
    }
    printf("end.\n");
    //release the memory.
    release(buffer);

    return 0;
}
static void usage()
{
    printf("Usage:\n");
    printf("\t[-a] # preamble length [1 to 8].Default 1.\n");
    printf("\t[-b] # preamble [1 to 8 bytes].Default \'0xAA\'.\n");
    printf("\t[-c] # mac address [1 to 5].Default 5.\n");
    printf("\t[-d] # if use esb [1 yes, 0 no].Default 1.\n");
    printf("\t[-e] # pcf len. Default 2.\n");
    printf("\t[-f] # crc len. Default 2.\n");
    printf("\t[-x] # slot number. Default 16.\n");
    printf("\t[-y] # period per signal.Deafult 7(ms).\n");
    printf("\t[-z] # signal file. Default \'data/4M_5743_recive_0.5_11_29.iq\'.\n");
    printf("\t[-s] # sample rate. Deafult 1MHz.\n");
    printf("\t[-h] # Display this text.\n");
}

int parse_opt(int argc, char *argv[], packet_param *pp)
{
    int opt;
    int result;
    
    while( (opt = getopt(argc, argv, "ha:b:c:d:e:f:x:y:z:s:")) != EOF)
    {
       switch( opt )
       {
           case 'h' :
               usage();
               exit(0);
           case 'a' :
               pp->preamble_len = (uint8_t)atoi(optarg);
               break;
           case 'b' :
               result = parse_u64(optarg, &pp->dest_preamble);
               break;
           case 'c' :
               pp->address_len = (uint8_t)atoi(optarg);
               break;
           case 'd' :
               pp->is_use_pcf = (uint8_t)atoi(optarg);
               break;
           case 'e' :
               pp->pcf_len = (uint8_t)atoi(optarg);
               break;
           case 'f' :
               pp->crc_len = (uint8_t)atoi(optarg);
               break;
           case 'x' :
               result = parse_u32(optarg, &pp->slot_number);
               break;
           case 'y' :
               result = parse_u32(optarg, &pp->period);
               break;
           case 'z' :
               sig_file = optarg;
               break;
           case 's':
               result = parse_u64(optarg, &pp->sample_rate);
               break;
           default :
               fprintf(stderr, "unknown argument '-%c %s'", opt, optarg);
               usage();
               return EXIT_FAILURE;
       }
    }
    return result;
    
}

int parse_u32(char* s, uint32_t* const value) {
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
