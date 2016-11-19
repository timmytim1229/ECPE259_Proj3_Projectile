#ifndef SD_H
#define SD_H
#include <stdint.h>
#include <ctype.h>

#define SD_BLOCKSIZE 512
#define SD_BLOCKSIZE_NBITS 9

uint8_t microSDInit( void );
uint8_t microSDRead (uint32_t blockaddr, uint8_t *data);
uint8_t microSDWrite (uint32_t blockaddr, uint8_t *data);

uint8_t sd_status ( void );
static uint8_t sd_initialize( void );
static void sd_wait_notbusy ( void );
static uint8_t sd_send_command( uint8_t cmd, uint8_t response_type, uint8_t crc);
static void sd_delay(uint8_t number);
static uint8_t sd_set_blocklen (uint32_t length);
#endif //  SD_H
