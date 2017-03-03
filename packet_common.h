
#include<stdint.h>

#define BYTE_TO_BITS    8

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
    .sample_rate = 1000000 \
};
