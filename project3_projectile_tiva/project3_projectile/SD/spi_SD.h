/*
 * spi_SD.h
 *
 *  Created on: Nov 16, 2016
 *      Author: Curtis
 */
#include <stdbool.h>

#ifndef SD_SPI_SD_H_
#define SD_SPI_SD_H_

void SPIinit();
void SSI0clockSpeed( bool fast );
void SSI0CSassert( void );
void SSI0CSdeassert( void );
void SSI0TxByte( uint8_t data );
void SSI0TxBuffer(uint16_t len, uint8_t *buffer);
uint8_t SSI0RxByte( void );
void SSI0RxEmpty( void );
void SSI0RxEmpty( void );
void SSI0RxBuffer(uint16_t len, uint8_t *buffer);

#endif /* SD_SPI_SD_H_ */
