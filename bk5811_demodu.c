//
//  main.c
//  decode_bk5811
//
//  Created by ddvv on 16/05/18.
//  Copyright © 2016年 ddvv. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#define DLT                 10.0

#define BK_SUCCESS          1
#define BK_FAILED           0
#define BK_OVERFLOW         4294967295
#define BK_PREAMBLE_BITS    (1*8)
#define BK_ADDRESS_BITS     (5*8)
#define BK_PCF_BITS         9
#define BK_PAYLOAD_BITS     (32*8)
#define BK_CRC_BITS         (2*8)


// save the signal
// odd：real
// eve: imag
char *g_buffer = NULL;

// file size
long g_file_length = 0;

// threshold
float g_threshold = 0.0;

#define SIGNAL_MAX_BYTES    (1+5+2+32+2)
#define SIGNAL_MAX_BITS     SIGNAL_MAX_BYTES * 8
#define SAMPLE_PER_SYMBOL   4

#define PACKET_COUNT 1000
// inter_array : less than 1000
// odd:start
// eve:end
long g_inter[PACKET_COUNT] = {0};

// read signal from file
// g_buffer : malloc in this function, and should be freed by release() function
int get_signal_data(char *filename)
{
    FILE *fp = NULL;
    
    fp = fopen(filename,"r");
    
    if(NULL == fp)
    {
        printf("open file failed.\n");
        //printf ("error: %s\n",strerror(errno));
        return BK_FAILED;
    }
    
    fseek(fp,0,SEEK_END);   // 2
    g_file_length = ftell(fp);
    fseek(fp,0,SEEK_SET);   // 0
    
    g_buffer = (char *)malloc(g_file_length);
   
    if(NULL == g_buffer)
    {
        printf("malloc mem failed.\n");
        return BK_FAILED;
    }
    
    // file should be less than 2^32
    fread(g_buffer, sizeof(char), g_file_length, fp);
    //printf ("error: %s\n",strerror(errno));
    
    fclose(fp);
    
    return  BK_SUCCESS;
}

// free the buffer
void release()
{
    if(NULL != g_buffer)
    {
        free(g_buffer);
        g_buffer = NULL;
    }
}

// find the threshold
int mean()
{
    unsigned long sum = 0;
    //g_file_length
    for(int i = 0; i < g_file_length; i += 2)
    {
        sum += abs((int8_t)g_buffer[i]);
        if(BK_OVERFLOW < sum)
        {
            printf("error : the sum is overflow!\n");
            return BK_FAILED;
        }
    }
    g_threshold = sum * 2.0 * DLT / g_file_length;
    return BK_SUCCESS;
}


// find the signal
int find_inter()
{
    long index = 0;
    int is_find = 0;
    int sample_count = 8;
    
    for(int i = 0; i < g_file_length; i+=2)
    {
        float sum_temp = 0.0;
        for(int j = 0; j < sample_count * 2; j += 2)
        {
            sum_temp += abs((int8_t)g_buffer[i + j]);
        }
        float mean_temp = sum_temp / sample_count;
        
        // find the signal start position
        if(mean_temp > g_threshold && 0 == is_find)
        {
            g_inter[index++] = i;
            is_find = 1;
        }
        // find the signal end position
        else if(mean_temp < g_threshold && 1 == is_find)
        {
            g_inter[index++] = i + sample_count * 2;
            is_find = 0;
        }
    }
    return BK_SUCCESS;
}

// demodulate the signal
int8_t demod_bits(long ss, int demod_length, int sample_per_symbol)
{
    int8_t result = 0;
    int8_t I0, Q0, I1, Q1;
    
    for(int i = 0;i < (demod_length * sample_per_symbol * 2 - 1); i += (sample_per_symbol*2))
    {
        I0 = g_buffer[ss + i];
        Q0 = g_buffer[ss + i + 1];
        I1 = g_buffer[ss + i + 2];
        Q1 = g_buffer[ss + i + 3];
        
        if((I0*Q1 - I1*Q0) > 0)
            result |= 1 << (demod_length - i/sample_per_symbol/2 - 1);
        else
            result |= 0 << (demod_length - i/sample_per_symbol/2 - 1);
        
    }
    
    return result;
}

// search the preamble
long search_preamble(long ss, long sig_len, int match_length, int sample_per_symbol)
{
    uint8_t result = 0;
    long sig_new_start = -1;
    uint8_t bit = 0;
    for(int i = 0; i < (sig_len - SIGNAL_MAX_BITS); i += 2)
    {
        // find 10101010b ＝ 0x0AA or 01010101b = 0x55
        result = demod_bits(ss + i, match_length, sample_per_symbol);
        bit = demod_bits(ss + i + match_length * sample_per_symbol * 2, 8, sample_per_symbol);
        bit >>= 7;
        bit &= 1;
        if((result == 0xAA) && (1 == bit))     // should be change by user
        {
            sig_new_start = i;
            break;
        }
        /*
        else if((result == 0x55) && (0 == bit))
        {
            sig_new_start = i;
            break;
        }
        */
    }
    return sig_new_start;
}

// value to bytes array
void packet_pack(int64_t address, uint16_t pcf, uint8_t *payload, int payload_len, uint8_t *packet)
{
    int c;
    uint64_t packet_header = address;
    packet_header <<= 9;
    packet_header |= pcf;
    for(c = 0; c < 7; c++)
        packet[c] = (packet_header >> ((6-c)*8)) & 0xff;
    for(c = 0; c < payload_len; c++)
        packet[c + 7] = payload[c];
}

// check the crc, if the crc not match, disable the signal
uint32_t calc_crc(const uint8_t *data, size_t data_len)
{
    uint8_t i;
    bool bit;
    uint8_t c;
    uint_fast16_t crc = 0x3C18;
    while(data_len--)
    {
        c = *data++;
        for(i = 0x80; i > 0; i >>= 1)
        {
            bit = crc & 0x8000;
            if(c & i)
                bit = !bit;
            crc <<= 1;
            if(bit)
                crc ^= 0x1021;
        }
        crc &= 0xffff;
    }
    return (uint16_t)(crc & 0xffff);
}

// work function
void work()
{
    int i = 0;
    uint32_t signal_count = 0;
    
    while(0 != g_inter[i])
    {
        long signal_start = g_inter[i];
        long signal_end = g_inter[i + 1];
        long signal_new_start = 0;
        uint8_t preamble = 0;
        int64_t address = 0;
        uint16_t pcf = 0;
        int payload_len = 0;
        uint8_t packet_buffer[32] = {0};
        uint16_t crc = 0;
        uint8_t packet[45] = {0};
        uint16_t new_crc = 0;
        // a signal must be more than 400bit
        if((signal_end - signal_start) > SIGNAL_MAX_BITS)
        {
            //find the preamble code
            signal_new_start = search_preamble(signal_start, signal_end - signal_start, 8, SAMPLE_PER_SYMBOL);
            if(-1 != signal_new_start)
            {
                // decode preamble
                signal_start += signal_new_start;
                preamble = demod_bits(signal_start, 8, SAMPLE_PER_SYMBOL);
                
                // decode address
                for (int j = 0; j < 5; j++) {
                    signal_start += (8 * SAMPLE_PER_SYMBOL * 2);
                    address <<= 8;
                    address |= (demod_bits(signal_start, 8, SAMPLE_PER_SYMBOL) & 0xff);
                }
                
                signal_count++;
                // decode pcf
                signal_start += (8 * SAMPLE_PER_SYMBOL * 2);
                pcf |= (uint8_t)demod_bits(signal_start, 8, SAMPLE_PER_SYMBOL);
                pcf <<= 1;
                signal_start += (8 * SAMPLE_PER_SYMBOL * 2);
                uint8_t temp = demod_bits(signal_start, 8, SAMPLE_PER_SYMBOL);
                temp >>= 7;
                pcf |= temp;
                
                // paylaod length must be less than 32
                payload_len = (pcf&0x1f8)>>3;
                
                if(payload_len <= 0x20)
                {
                    // decode payload
                    signal_start += (1 * SAMPLE_PER_SYMBOL * 2);
                    for (int j = 0; j < payload_len; j++) {
                        packet_buffer[j] = demod_bits(signal_start, 8, SAMPLE_PER_SYMBOL);
                        signal_start += (8 * SAMPLE_PER_SYMBOL * 2);
                    }
                
                    // decode crc
                    //signal_start += (8 * SAMPLE_PER_SYMBOL * 2);
                    crc = demod_bits(signal_start, 8, SAMPLE_PER_SYMBOL);
                    signal_start += (8 * SAMPLE_PER_SYMBOL * 2);
                    crc <<= 8;
                    crc |= (demod_bits(signal_start, 8, SAMPLE_PER_SYMBOL)&0xff);
                    
                    packet_pack(address, pcf, packet_buffer, payload_len, packet);
                    new_crc = calc_crc(packet, payload_len + 7);
                
                    //
                    if(crc == new_crc)
                    {
                        // print the values
                        printf("pk_count : %d,\tpreamble : %X,\taddress : %05llX,\tpayload length : %d,\tpid : %d,\tno_ack : %d,\t", signal_count, preamble, address, (pcf&0x1f8)>>3, (pcf&0x6)>>1,pcf&1);
                        printf("payload : ");
                        for(int j = 0; j < payload_len; j++)
                            printf("%02X", packet_buffer[j]);
                        printf(",\tcrc : %04X\n", crc);
                    }
                }
            }
        }
        signal_new_start = -1;
        i += 2;
    }
}



int main(int argc, const char * argv[]) {
    // read signal
    //char *sig_file = "4M_5743_recive_0.5s.iq";
    char *sig_file = "4M_5743_recive_0.5_11_29.iq";
    if(argc == 2)
        sig_file = (char *)argv[1];
    get_signal_data(sig_file);
    mean();
    find_inter();
    work();
    
    // release the mem
    release();
    return 0;
}
