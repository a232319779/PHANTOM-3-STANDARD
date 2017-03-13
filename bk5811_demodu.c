//
//  main.c
//  decode_bk5811
//
//  Created by ddvv on 16/05/18.
//  Copyright © 2016年 ddvv. All rights reserved.
//

#include "bk5811_demodu.h"


// threshold
float g_threshold = 0.0;
#define DEFAULT_THRESHOLD   (10.0)

// inter_array : less than 1000
// odd:start
// eve:end
long g_inter[PACKET_COUNT] = {0};

int g_pkg_count = 0;

// get file size
long get_file_size(char* filename)
{
    FILE *fp = NULL;
    long file_length = -1;
    
    fp = fopen(filename,"r");
    
    if(NULL == fp)
    {
        printf("open file failed.\n");
        //printf ("error: %s\n",strerror(errno));
        return BK_FAILED;
    }
    
    fseek(fp,0,SEEK_END);   // 2
    file_length = ftell(fp);
    fseek(fp,0,SEEK_SET);   // 0

    fclose(fp);
    
    return file_length;
}

// read signal from file
// buffer : malloc in this function, and should be freed by release() function
int get_signal_data( IN char *filename, IN char *buffer, IN long read_offset, IN OUT long *read_length)
{
    FILE *fp = NULL;
    
    fp = fopen(filename,"r");
    
    if(NULL == fp)
    {
        printf("open file failed.\n");
        //printf ("error: %s\n",strerror(errno));
        return BK_FAILED;
    }

    fseek(fp, read_offset, SEEK_SET);
    
    // file should be less than 2^32
    *read_length = fread(buffer, sizeof(char), *read_length, fp);
    //printf ("error: %s\n",strerror(errno));
    
    fclose(fp);
    
    return  BK_SUCCESS;
}

// free the buffer
void release(IN char *buffer)
{
    if(NULL != buffer)
    {
        free(buffer);
        buffer = NULL;
    }
}

// find the threshold
float mean(IN char *buffer, IN long start, IN long length)
{
    unsigned long sum = 0;
    
    long end = start + length;

    for(long i = start; i < end; i += 2)
    {
        sum += abs((int8_t)buffer[i]);
        if(BK_OVERFLOW < sum)
        {
            printf("error : the sum is overflow!\n");
            return BK_FAILED;
        }
    }
    float temp = (sum * 2.0 * DLT / length);
    //g_threshold = temp > DEFAULT_THRESHOLD ? temp : DEFAULT_THRESHOLD;
    g_threshold = temp;

    return temp;
//    return BK_SUCCESS;
}


// find the signal
int find_inter(IN char *buffer, IN long start, IN long length)
{
    long index = 0;
    int is_find = 0;
    int sample_count = 8;
    
    long end = start + length;
    for(long i = start; i < end - sample_count * 2; i+=2)
    {
        float sum_temp = 0.0;
        for(long j = 0; j < sample_count * 2; j += 2)
        {
            sum_temp += abs((int8_t)buffer[i + j]);
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
            is_find = 2;
        }
        if(2 == is_find)
        {
            if((g_inter[index-1] - g_inter[index-2]) < SIGNAL_MAX_BITS)
                index -=2;
            is_find = 0;
        }
    }
    return BK_SUCCESS;
}

// demodulate the signal
// 使用有符号数没有问题。有些信号crc校验不过，可能是因为接收器的误差，导致有符号位翻转等原因。
int8_t demod_bits(IN char *buffer, IN long ss, IN int demod_length, IN int sample_per_symbol)
{
    int8_t result = 0;
    int8_t I0, Q0, I1, Q1;
    
    for(int i = 0;i < (demod_length * sample_per_symbol * 2 - 1); i += (sample_per_symbol * 2))
    {
        I0 = buffer[ss + i];
        Q0 = buffer[ss + i + 1];
        I1 = buffer[ss + i + 2];
        Q1 = buffer[ss + i + 3];
        
        if((I0*Q1 - I1*Q0) > 0)
            result |= 1 << (demod_length - i/sample_per_symbol/2 - 1);
        else
            result |= 0 << (demod_length - i/sample_per_symbol/2 - 1);
        
    }
    
    return result;
}

// search the preamble
long search_preamble(IN char *buffer, IN long ss, IN long sig_len, IN int match_length, IN int preamble_bytes, IN uint64_t dest_preamble, IN int sample_per_symbol)
{
    uint64_t result = 0;
    long sig_new_start = -1;
    int8_t preamble_len = 0;
    int j;
    
    for(int i = 0; i < (sig_len - SIGNAL_MAX_BITS); i += 2)
    {
        j = i;
        result = 0;
        preamble_len = preamble_bytes;
        while(preamble_len--)
        {
            result <<= 8;
            result |= (uint8_t)demod_bits(buffer, ss + j, match_length, sample_per_symbol);
            j += (2 * 8 * sample_per_symbol);
        }
        if(result == dest_preamble) 
        {
            sig_new_start = i;
            break;
        }
    }
    return sig_new_start;
}

// value to bytes array
void packet_pack(IN int64_t address, IN uint16_t pcf, IN uint8_t *payload, IN int payload_len, OUT uint8_t *packet)
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
uint32_t calc_crc(IN const uint8_t *data, IN size_t data_len)
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
int work(IN char *buffer, IN packet_param *lpp, OUT long *start_position,OUT uint8_t *channel)
{
    int i = 0;
    int isfind = 0;
    
    // test 
    int count = 0;
    while(0 != g_inter[i])
    {
        long signal_start = g_inter[i];
        long signal_end = g_inter[i + 1];
        long signal_new_start = 0;
        uint64_t preamble = 0;
        int64_t address = 0;
        uint16_t pcf = 0;
        int payload_len = 0;
        uint8_t packet_buffer[32] = {0};
        uint16_t crc = 0;
        uint8_t packet[45] = {0};
        uint16_t new_crc = 0;
        long tmp_start = 0;
        int sample_per_symbol = lpp->sampler_per_symbol;

        // a signal must be more than 400bit
        if((signal_end - signal_start) > SIGNAL_MAX_BITS)
        {
            //find the preamble code
            signal_new_start = search_preamble(buffer, signal_start, signal_end - signal_start, 8, lpp->preamble_len, lpp->dest_preamble, sample_per_symbol);
            if(-1 != signal_new_start)
            {
                // decode preamble
                signal_start += signal_new_start;
                tmp_start = signal_start;

                int preamble_len = lpp->preamble_len;
                while(preamble_len--)
                {
                    preamble <<= 8;
                    preamble |= (demod_bits(buffer, signal_start, 8, sample_per_symbol) & 0xff);
                    signal_start += (8 * sample_per_symbol * 2);
                }
                
                // decode address
                for (uint8_t j = 0; j < lpp->address_len; j++) {
                    address <<= 8;
                    address |= (demod_bits(buffer, signal_start, 8, sample_per_symbol) & 0xff);
                    signal_start += (8 * sample_per_symbol * 2);
                }
                
                // decode pcf
                if(lpp->is_use_pcf == 1)
                {
                    pcf |= (demod_bits(buffer, signal_start, 8, sample_per_symbol) & 0xff);
                    signal_start += (8 * sample_per_symbol * 2);
                    pcf <<= 1;
                    uint8_t temp = (demod_bits(buffer, signal_start, 8, sample_per_symbol) & 0xff);
                    temp >>= 7;
                    pcf |= temp;
                    
                    // paylaod length must be less than 32
                    payload_len = (pcf&0x1f8)>>3;
                }
                if(payload_len <= 0x20)
                {
                    // decode payload
                    signal_start += (1 * sample_per_symbol * 2);
                    for (int j = 0; j < payload_len; j++) {
                        packet_buffer[j] = (demod_bits(buffer, signal_start, 8, sample_per_symbol) & 0xff);
                        signal_start += (8 * sample_per_symbol * 2);
                    }
                
                    // decode crc
                    uint8_t crc_len = lpp->crc_len;
                    while(crc_len--)
                    {
                        crc <<= 8;
                        crc |= (demod_bits(buffer, signal_start, 8, sample_per_symbol) & 0xff);
                        signal_start += (8 * sample_per_symbol * 2);
                    }
                    packet_pack(address, pcf, packet_buffer, payload_len, packet);
                    new_crc = calc_crc(packet, payload_len + 7);
                
                    //
                    if(crc == new_crc)
                    {
                        *start_position = tmp_start;
                        *channel = 255 - ((address>>32)&0xff);
                        count++;
                        //printf("find %d signal.\n", count);
                        g_pkg_count++;
                        // print the values
                        printf("channel : %d,\tpk_count : %d,\tpreamble : %llX,\taddress : %05llX,\tpayload length : %d,\tpid : %d,\tno_ack : %d,\t", *channel, g_pkg_count, preamble, address, (pcf&0x1f8)>>3, (pcf&0x6)>>1,pcf&1);
                        printf("payload : ");
                        for(int j = 0; j < payload_len; j++)
                            printf("%02X", packet_buffer[j]);
                        printf(",\tcrc : %04X\n", crc);
                        // find signal
                        isfind = 1;
                    }
#if 0
                    else
                    {
                        FILE *fd = NULL;
                        char filename[255] = {0};
                        sprintf(filename, "data/could_not_demodule_%ld.iq", tmp_start);
                        fd = fopen(filename, "wb");
                        int file_dlt = 1000;
                        long len = g_inter[i+1] - g_inter[i] + file_dlt * 2;
                        fwrite(&buffer[g_inter[i]] - file_dlt, 1, len, fd);
                        fclose(fd);
                    }
#endif
                }
            }
        }
        signal_new_start = -1;
        i += 2;
    }

    return isfind;
}
/*
 * debug function
 *
*/
// debug the could not demodule signal.
void set_inter(long end)
{
    g_inter[0] = 2;
    g_inter[1] = end;
}
