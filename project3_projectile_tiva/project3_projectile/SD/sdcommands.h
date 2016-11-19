#ifndef SDCOMMANDS_H
#define SDCOMMANDS_H
//#include <std_Types.h>
#include <ctype.h>

#define R1 1
#define R1B 2
#define R2 3
#define R3 4
#define R7 5

#define MSK_IDLE 0x01
#define MSK_TOK_DATAERROR 0xE0

// Mask off the bits in the OCR corresponding to voltage range 3.2V to
// 3.4V, OCR bits 20 and 21 
#define MSK_OCR_33 0xC0

// Number of times to retry the probe cycle during initialization
#define SD_INIT_TRY 50

// Number of tries to wait for the card to go idle during initialization
#define SD_IDLE_WAIT_MAX 100

// Hardcoded timeout for commands. 8 words, or 64 clocks. Do 10 words instead
#define SD_CMD_TIMEOUT 100

// Basic command set
// Reset cards to idle state
#define CMD0 0
#define CMD0_R R1
//
#define CMD1 1
#define CMD1_R R1
// Card sends the CSD
#define CMD8 8
#define CMD8_R R7

// Card sends the CSD
#define CMD9 9
#define CMD9_R R1
// Card sends CID
#define CMD10 10
#define CMD10_R R1

#define CMD13 13
#define CMD13_R R2


// Block read commands
// Set the block length */
#define CMD16 16
#define CMD16_R R1
// Read a single block */
#define CMD17 17
#define CMD17_R R1

// Write block
#define CMD24 24
#define CMD24_R R1B //not sure about this one

// Application-specific commands
// Flag that the next command is application-specific
#define CMD55 55
#define CMD55_R R1
// Read the OCR (SPI mode only)
#define CMD58 58
#define CMD58_R R3

// Application-specific commands
// Get the card's OCR (SD mode)
#define ACMD41 41
#define ACMD41_R R1

#endif // SDCOMMANDS_H
