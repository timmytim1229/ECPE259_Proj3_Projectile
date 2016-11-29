/*
 * writesd.c
 *
 *  Created on: Nov 28, 2016
 *      Author: krackletopwin
 */

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_i2c.h"
#include "inc/hw_gpio.h"
#include <inc/hw_memmap.h>			// for ADC & UART0
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <driverlib/sysctl.h>
#include <driverlib/adc.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/uart.h>
#include <driverlib/i2c.h>

#include "diskio.h"
#include "ff.h"
#include "microSD.h"
#include "spi_SD.h"

void writeSDchar(char data, FATFS FatFs, FIL LogFile, FRESULT Res)
{
	f_mount(&FatFs, "", 0); //Mount volume
	f_open(&LogFile, "test.log", FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&LogFile, f_size(&LogFile));

	//f_putc(data, &LogFile);
	f_printf(&LogFile, "%c", data);
	f_close(&LogFile);
}

void writeSD(uint8_t data, FATFS FatFs, FIL LogFile, FRESULT Res)
{
	f_mount(&FatFs, "", 0); //Mount volume
	f_open(&LogFile, "test.log", FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&LogFile, f_size(&LogFile));

	//f_putc(data, &LogFile);
	f_printf(&LogFile, "%x", data);
	f_close(&LogFile);
}

void writeSDMPU(char *data, FATFS FatFs, FIL LogFile, FRESULT Res)
{
	f_mount(&FatFs, "", 0); //Mount volume
	f_open(&LogFile, "test.log", FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&LogFile, f_size(&LogFile));

	//uint8_t written;
	//f_putc(data, &LogFile);

	f_puts(data, &LogFile);
	f_close(&LogFile);
}
