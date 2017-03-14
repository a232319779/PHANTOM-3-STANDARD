#ifndef _PACKET_COMMON_H_
#define _PACKET_COMMON_H_

#include<stdint.h>

#define BYTE_TO_BITS    8

#define ONE_MHZ     1000000llu

#define START_FREQ  (5725llu * ONE_MHZ)
#define CHANNELS_NUMBERS    127

#ifndef DEFAULT_SAMPLE
#define DEFAULT_SAMPLE (ONE_MHZ * 1) 
#endif

// packet
typedef struct _PACKET_{
    // ESB packet
    uint64_t preamble;
    uint64_t address;
    uint16_t payload_len;
    uint8_t pid;
    uint8_t no_ack;
    uint8_t packet_buffer[64];
    uint64_t crc;
    // ESB packet end
    uint8_t channel;
    long start_position;
}s_packet;

// packet param
typedef struct _PACKET_PARAM{
    uint8_t preamble_len;       /*  byte    */
    uint64_t dest_preamble;     /*  match preamble  */
    uint8_t address_len;        /*  byte    */
    uint8_t is_use_pcf;         /*  bool 1:use 0:not use   */
    uint8_t pcf_len;            /*  byte    */
    uint8_t crc_len;            /*  byte    */
    uint32_t slot_number;       /*  hopping number  */
    uint32_t period;            /*  ms  */
    uint64_t sample_rate;       /*  hz  */
    int sampler_per_symbol;     /*  per/MHz */
    uint8_t size_per_channel;   /*  MHz */
    uint64_t start_freq;        /*  which frequence start to scan : MHz*/
    uint32_t channel_number;    /*  how many channel should be scanned.*/
}packet_param;

#define INIT_PP() { \
    .preamble_len = 1, \
    .dest_preamble = 0xAA, \
    .address_len = 5, \
    .is_use_pcf = 1, \
    .pcf_len = 2, \
    .crc_len = 2, \
    .slot_number = 16, \
    .period = 7, \
    .sample_rate = DEFAULT_SAMPLE, \
    .sampler_per_symbol = (DEFAULT_SAMPLE / ONE_MHZ), \
    .size_per_channel = 1, \
    .start_freq = START_FREQ, \
    .channel_number = CHANNELS_NUMBERS \
};

// global pp
//packet_param pp = INIT_PP();

#endif
