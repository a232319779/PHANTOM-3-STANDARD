#include <memory.h>
#include <getopt.h>
#include "bk5811_demodu.h"
#include "parse_opt.h"

static void usage();
int parse_opt(int argc, char *argv[]);

extern packet_param pp;


int main(int argc, char *argv[])
{
    s_packet *sp = NULL;
    decode_param *dp = NULL;

    // default signal file
    rp.path = "data/1M_5738_recive_1s.iq";

    if (argc > 1)
    {
        if(-2 == parse_opt(argc, argv))
            return -1;
    }
    char *buffer = NULL;
    long file_length = 0;
    long read_length = 0;
    long read_offset = 0;
    long size_per_period;
    float diff_time;
    long start_p = 0;
    float period = 99999999.9;

    dp = (decode_param *)malloc(sizeof(decode_param));
    sp = (s_packet *)malloc(sizeof(s_packet));

    size_per_period = (pp.sample_rate * pp.slot_number * pp.period * 2 / 1000);
    file_length = get_file_size(rp.path);
    read_length = size_per_period * 1;
    buffer = (char *)malloc(read_length);
    while(read_offset < file_length)
    {
        // init
        memset(dp, 0, sizeof(decode_param));
        memset(buffer, 0, read_length);
        
        // read data
        get_signal_data(rp.path, buffer, read_offset, &read_length);
        read_offset += read_length;

        // get threshold and inter[]
        mean(buffer, 0, read_length, dp);
        find_inter(buffer, 0, read_length, dp);

        diff_time = 0.0;
        // demodulate
        while(dp->current < dp->total)
        {
            memset(sp, 0, sizeof(s_packet));

            int iret =  work(buffer, dp, &pp, sp);
            if(1 == iret)
            {
                printf("channel : %d,\tpreamble : %llX,\taddress : %05llX,\tpayload length : %d,\tpid : %d,\tno_ack : %d,\t", sp->channel, sp->preamble, sp->address, sp->payload_len, sp->pid, sp->no_ack);
                printf("payload : ");
                for(int j = 0; j < sp->payload_len; j++)
                    printf("%02X", sp->packet_buffer[j]);
                printf(",\tcrc : %llX\n", sp->crc);

                if(start_p != 0)
                {
                    diff_time = calc_diff_time(start_p, sp->start_position + read_offset, pp.sample_rate);
                    //printf("diff_time : %f\n", diff_time);
                    if(diff_time < period)
                        period = diff_time;
                }
                start_p = sp->start_position + read_offset;
            }
            dp->current += 2;
        }
    }
    printf("!!! : %fms may be the period.\n", period);
    printf("end.\n");

    //release the memory.
    release(buffer);
    free(dp);
    dp = NULL;
    free(sp);
    sp = NULL;

    return 0;
}

static void usage()
{
    printf("Usage:\n");
    printf("\t[-h] # Display this text.\n");
    printf("\t[-i] # preamble length [1 to 8].Default 1.\n");
    printf("\t[-j] # preamble [1 to 8 bytes].Default \'0xAA\'.\n");
    printf("\t[-m] # mac address [1 to 5].Default 5.\n");
    printf("\t[-e] # if use esb [1 yes, 0 no].Default 1.\n");
    printf("\t[-p] # pcf len. Default 2.\n");
    printf("\t[-c] # crc len. Default 2.\n");
    printf("\t[-t] # slot number. Default 16.\n");
    printf("\t[-y] # period per signal.Deafult 7(ms).\n");
    printf("\t[-r] # signal file. Default \'data/1M_5748_recive_1s.iq\'.\n");
    printf("\t[-s] # sample rate. Deafult 1MHz.\n");
    printf("Default set : -i 1 -j 0xAA -m 5 -e 1 -p 2 -c 2 -t 16 -y 7 -s 1000000 -r data/1M_5738_recive_1s.iq\n");
}
int parse_opt(int argc, char *argv[])
{

    char *opt_selcet = "hi:j:m:e:p:c:t:y:r:s:";
    parse_opt_param(argc, argv, opt_selcet, usage);

    return 0;
}
