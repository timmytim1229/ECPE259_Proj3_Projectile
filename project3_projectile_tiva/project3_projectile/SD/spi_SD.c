/*
 * spi_SD.c
 *
 *  Created on: Nov 16, 2016
 *      Author: Curtis
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"

/* Initialize and enable the SPI module */

void SPIinit() {

	// GPIO Initialization
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	GPIOPinConfigure(GPIO_PA2_SSI0CLK);
	GPIOPinConfigure(GPIO_PA4_SSI0RX);
	GPIOPinConfigure(GPIO_PA5_SSI0TX);
	//GPIOPinConfigure(GPIO_PA3_SSI0FSS);
	GPIOPinTypeSSI(GPIO_PORTA_BASE, (GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5));

	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);

	// SPI Initialization
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0));

	SSIDisable(SSI0_BASE);
	SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 400000, 8);
	SSIEnable(SSI0_BASE);

	uint32_t data;

	while(SSIDataGetNonBlocking(SSI0_BASE, &data));

	while(SSIBusy(SSI0_BASE));
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);
}

// Set SPI clock speed to fast (12.5MHz) or slow (400kHz)
void SSI0clockSpeed( bool fast ) {
	SSIDisable(SSI0_BASE);
	if (fast)
		SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,	SSI_MODE_MASTER, 5000000, 8);
	else
		SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,	SSI_MODE_MASTER, 400000, 8);
	SSIEnable(SSI0_BASE);
	while(SSIBusy(SSI0_BASE));
}

// Assert CS, active low
void SSI0CSassert( void ) {
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);
}

void SSI0CSdeassert( void )
{
	while(SSIBusy(SSI0_BASE));
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);
}

/* Send a single byte over the SPI port */
void SSI0TxByte( uint8_t data )
{
	SSIDataPut(SSI0_BASE, data);
	while(SSIBusy(SSI0_BASE));
	uint32_t x;
	while(SSIDataGetNonBlocking(SSI0_BASE, &x));
}

void SSI0TxBuffer(uint16_t len, uint8_t *buffer) {
	int i;
	for(i = 0; i < len; i++) {
		SSIDataPut(SSI0_BASE, buffer[i]);
	}
	uint32_t x;
	while(SSIDataGetNonBlocking(SSI0_BASE, &x));
}

uint8_t SSI0RxByte( void ) {
	uint32_t data;
	SSIDataPut(SSI0_BASE, 0xff);
	SSIDataGet(SSI0_BASE, &data);
	return data;
}

void SSI0RxEmpty( void )
{
	uint32_t x;
	while(SSIDataGetNonBlocking(SSI0_BASE, &x));
}

void SSI0RxBuffer(uint16_t len, uint8_t *buffer) {
	int i;
	for (i = 0; i < len; i++) {
		uint32_t data;
		SSIDataPut(SSI0_BASE, 0xff);
		SSIDataGet(SSI0_BASE, &data);
		buffer[i] = data;
	}
}
