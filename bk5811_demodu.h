/*
 *
 *
 *
 *
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef BK5811_DEMODU_H
#define BK5811_DEMODU_H
#endif

#define DLT                 10.0

#define BK_SUCCESS          1
#define BK_FAILED           0
#define BK_OVERFLOW         4294967295
#define BK_PREAMBLE_BITS    (1*8)
#define BK_ADDRESS_BITS     (5*8)
#define BK_PCF_BITS         9
#define BK_PAYLOAD_BITS     (32*8)
#define BK_CRC_BITS         (2*8)


#define SIGNAL_MAX_BYTES    (1+5+2+32+2)
#define SIGNAL_MAX_BITS     SIGNAL_MAX_BYTES * 8
#define SAMPLE_PER_SYMBOL   4

#define PACKET_COUNT 1000

#define PER_PACKET_SIZE (4000000 * 7 * 2 / 1000)
#define PACKET_SIZE (2 * PER_PACKET_SIZE)

extern long g_inter[PACKET_COUNT];

// read signal from file
// g_buffer : malloc in this function, and should be freed by release() function
int get_signal_data(char *filename, char **buffer, long *file_length);

// free the buffer
void release(char *buffer);

// calculate the threshold
int mean(char *buffer, long start, long length);

// find the signal
int find_inter(char *buffer, long start, long length);

// work function
// if find signal return 1, others 0.
int work(char *buffer);

/* pravite functions */

// dedmodulate the signal
int8_t demod_bits(char *buffer, long ss, int demod_length, int sample_per_symbol);

// search the preamble
long search_preamble(char *buffer, long ss, long sig_len, int match_length, int sample_per_symbol);

// value to bytes array
void packet_pack(int64_t address, uint16_t pcf, uint8_t *payload, int payload_len, uint8_t *packet);

// check the crc, if the crc not match, disable the signal
uint32_t calc_crc(const uint8_t *data, size_t data_len);

