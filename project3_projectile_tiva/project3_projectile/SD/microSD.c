#include <stdint.h>
#include <ctype.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"

#include "spi_SD.h"
#include "microSD.h"
#include "sdcommands.h"

static uint8_t response[5];
static int32_t argument_l;
static uint8_t SDHC;
uint8_t microSDstatus = 0;

const uint16_t sdc_timeout_read = 1000;

uint8_t sd_status( void )
{
	return microSDstatus;
}

static uint8_t sd_initialize( void )
{
// SPI SD initialization sequence:
// Delay for at least 74 clock cycles. This means to actually *clock* out 
// at least 74 clock cycles with no data present. In SPI mode, send at 
// least 10 idle bytes (0xFF).
  SSI0CSassert();
  //sd_delay(10);
  SSI0CSdeassert();
  sd_delay(10);

  // send CMD0 while CS is low to go into SPI mode
  SSI0CSassert();
  if (sd_send_command(CMD0, CMD0_R,0x95) == 0) return 0;
  //SSI0CSdeassert();
  SSI0TxByte(0xff);

  // send CMD8; required for Phys Spec V2.0 cards
  // NOTE: CRC must be valid for CMD8
  SSI0CSassert();
  argument_l = 0x000001aa;      // voltage range 2.7-3.3V, echo response 0xaa
  if (sd_send_command(CMD8, CMD8_R,0x87) == 0) return 0;
  //SSI0CSdeassert();
  // should check response here; it should echo back
  argument_l = 0x0;
  SSI0TxByte(0xff);

  /*
  // now send CMD58 to check operating voltage range
  SSI0CSassert();
  if (sd_send_command(CMD58, CMD58_R,0) == 0) return 0; // 0x6f
  //SSI0CSdeassert();
  SSI0TxByte(0xff);
  // check for valid range here
  */

  // send initial status command to ask for HCS (High Capacity Support)
  SSI0CSassert();
  argument_l = 0x00;
  if (sd_send_command(CMD55, CMD55_R,0) == 0) return 0;
  //SSI0CSdeassert();
  SSI0TxByte(0xff);
  SSI0CSassert();
  argument_l = 0x40000000;      // HCS bit
  if (sd_send_command(ACMD41, ACMD41_R,0) == 0) return 0;
  //SSI0CSdeassert();
  SSI0TxByte(0xff);
  argument_l = 0x00;

  // now wait for busy to clear
  do {
    SSI0CSassert();
    if (sd_send_command(CMD55, CMD55_R,0) == 0) return 0;
    //SSI0CSdeassert();
    SSI0TxByte(0xff);
    SSI0CSassert();
    if (sd_send_command(ACMD41, ACMD41_R,0) == 0) return 0;
    //SSI0CSdeassert();
    SSI0TxByte(0xff);
  } while ( response[0] & 0x01 );


  // now send CMD58 again; CCS should be set for High capacity cards
  SSI0CSassert();
  if (sd_send_command(CMD58, CMD58_R,0) == 0) return 0;
  //SSI0CSdeassert();
  SSI0TxByte(0xff);

  // find out whether card is 4GB or larger
  SDHC = (response[3] & 0x40) != 0;

  if( !SDHC ) {
  // for standard capacity cards, set block size
    SSI0CSassert();
    argument_l = SD_BLOCKSIZE;
    if (sd_send_command(CMD16, CMD16_R,0) == 0) return 0;
    //SSI0CSdeassert();
    SSI0TxByte(0xff);
    argument_l = 0;
  }
      // yea: initialization was OK.
  SSI0CSdeassert();
  microSDstatus = 1;
  return 1;
}

static uint8_t sd_send_command( uint8_t cmd, uint8_t response_type, uint8_t crc)
{
  int16_t i;
  int8_t response_length;
  uint8_t tmp;
  uint8_t *ptr = (uint8_t *)&argument_l;
  // All data is sent MSB first, and MSb first */
  // Send the header/command */
  // Format:
  //   cmd[7:6] : 01
  //   cmd[5:0] : command
  SSI0TxByte((cmd & 0x3F) | 0x40);
  i = 4;
  while ( i > 0 )
    SSI0TxByte(ptr[--i]);
  // This is the CRC. It only matters what we put here for the first
  // command. Otherwise, the CRC is ignored for SPI mode unless we
  // enable CRC checking.
  SSI0TxByte(crc);
  response_length = 0;
  switch (response_type)
  {
    case R1:
    case R1B:
      response_length = 1;
      break;
    case R2:
      response_length = 2;
      break;
    case R3:
    case R7:
    default:
      response_length = 5;
      break;
  }
  // Wait for a response. A response can be recognized by the start bit
  // (a zero)
  for( i = 0; i < SD_CMD_TIMEOUT*10; i++ )
  {
    tmp = SSI0RxByte();
    if ( ( tmp & 0x80 ) == 0 ) {
      while ( response_length > 0 ) {
        response[--response_length] = tmp;
        // This handles the trailing-byte requirement.
        tmp = SSI0RxByte();
      }
      // If the response is a "busy" type (R1B), then there's some special 
      // handling that needs to be done. The card will output a continuous 
      // stream of zeros, so the end of the BUSY state is signaled by any 
      // nonzero response. The bus idles * high.
      if ( response_type == R1B ) {
        do {
          tmp = SSI0RxByte();
        }
        while (tmp != 0xFF);
      }
      return 1;
    }
  }
  // If we make it here, we never got a response; deselect card
  SSI0CSdeassert();
  return 0;
}

static void sd_delay(uint8_t number)
{
  while( number != 0 ) {
    SSI0TxByte(0xFF);
    --number;
  }
  while(SSIBusy(SSI0_BASE));
}

// This function initializes the SD card. It returns 1 if initialization 
// was successful, 0 otherwise.

uint8_t microSDInit( void )
{
  uint8_t i = SD_INIT_TRY;
  SSI0clockSpeed ( false );
  while (sd_initialize() != 1) {
     --i;
     if (i == 0) return 0;
  }
  SSI0clockSpeed ( true );
  return 1;
}

// This function reads a single block from the SD card at block blockaddr. 
// The buffer must be preallocated. Returns 1 if the command was successful,
// zero otherwise.
// uint32_t blockaddr -- The block address to read from the card.
//    This is a block address, not a linear address.
// uint8_t *data -- The data, a pointer to an array of unsigned chars.

uint8_t microSDRead (uint32_t blockaddr, uint8_t *data)
{
  uint16_t i;
  uint8_t tmp;
  uint8_t count = 10;

  /* Adjust the block address to a linear address for standard cards */
  if( !SDHC ) blockaddr <<= SD_BLOCKSIZE_NBITS;
  while ( count ) {
    argument_l = blockaddr;
    SSI0CSassert();
    if (sd_send_command(CMD17, CMD17_R,0) == 0) return 0;
    /* Check for an error, like a misaligned read */
    if (response[0] != 0)
      continue;
    /* Re-assert CS to continue the transfer */

    /* Wait for the token */
    i=sdc_timeout_read;
    do {
      tmp = SSI0RxByte();
      i--;
    }
    while ((tmp == 0xFF) && i != 0); 
    // The card returned an error response. Bail and return 0
    if ((tmp & MSK_TOK_DATAERROR) == 0) {
      SSI0CSdeassert();
      // Clock out a byte before returning
      SSI0TxByte(0xFF);
      --count;
    } else {
      SSI0RxBuffer(SD_BLOCKSIZE, data);
      SSI0CSdeassert();
      return 1;
    }
  }
  return 0;
    // now do the actual buffer read
}



/* This function writes a single block to the SD card at block
blockaddr. Returns 1 if the command was successful, zero
otherwise.
This is an ASYNCHRONOUS call. The transfer will not be complete
when the function returns. If you want to explicitly wait until
any pending transfers are finished, use the command
sd_wait_notbusy(sdc).
sd_context_t *sdc -- a pointer to an sd device context structure,
populated by the function sd_initialize()
u32 blockaddr -- The block address to read from the card.
This is a block address, not a linear address.
unsigned char *data -- The data, a pointer to an array of unsigned
chars.
*/

uint8_t microSDWrite (uint32_t blockaddr, uint8_t *data)
{

  /* Adjust the block address to a linear address */
  if( !SDHC ) blockaddr <<= SD_BLOCKSIZE_NBITS;

  argument_l = blockaddr;
  SSI0CSassert();
  if (sd_send_command(CMD24, CMD24_R, 0) == 0) return 0;
    /* Check for an error, like a misaligned write */
  if (response[0] != 0) return 0;
  /* Re-assert CS to continue the transfer */
  //SSI0CSassert();

  /* The write command needs an additional 8 clock cycles before
  * the block write is started. */
  SSI0TxByte(0xfe);
  SSI0RxEmpty();
  SSI0TxBuffer(512, data);

  argument_l = 0;
  sd_send_command(CMD13, CMD13_R, 0);

  while(SSI0RxByte()==0x00);
  while(SSI0RxByte()!=0xFF);

  SSI0CSdeassert();

  return 1;
}

