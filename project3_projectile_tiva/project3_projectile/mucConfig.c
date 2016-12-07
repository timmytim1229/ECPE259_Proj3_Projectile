/*
 * config.c
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
#include "SD/diskio.h"
#include "SD/ff.h"
#include "SD/microSD.h"
#include "SD/spi_SD.h"
#include "SD/writeSD.h"

#include "i2cimu.h"

void UARTConfig(uint32_t sysctl_p_uart, uint32_t sysctl_p_gpio, uint32_t rxpin, uint32_t rxpin_num, uint32_t txpin, uint32_t txpin_num, uint32_t gpio_port_base, uint32_t uart_base, uint32_t baud_rate)
{
    // enable the UART
    SysCtlPeripheralEnable(sysctl_p_uart);
    SysCtlPeripheralEnable(sysctl_p_gpio);

    // Configure the GPIO pins as UART and output pins
    GPIOPinConfigure(rxpin);
    GPIOPinConfigure(txpin);
    GPIOPinTypeUART(gpio_port_base, (rxpin_num | txpin_num));

    // Configure the UART: 8-bit communication, 1 stop bit, no parity
    UARTConfigSetExpClk(uart_base, SysCtlClockGet(), baud_rate,
         UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
         UART_CONFIG_PAR_NONE);
}

void I2CConfig(uint32_t sysctl_p_i2c, uint32_t gpio_scl, uint32_t scl_pin_num, uint32_t gpio_sda, uint32_t sda_pin_num, uint32_t gpio_port_base, uint32_t i2c_base, bool bFast)
{
	// Enable the I2C module
    SysCtlPeripheralEnable(sysctl_p_i2c);
    // reset module
    SysCtlPeripheralReset(sysctl_p_i2c);

	// configure pins for I2C1 line
	GPIOPinConfigure(gpio_scl);
	GPIOPinConfigure(gpio_sda);
	GPIOPinTypeI2CSCL(gpio_port_base, scl_pin_num);
    GPIOPinTypeI2C(gpio_port_base, sda_pin_num);
    while(!SysCtlPeripheralReady(sysctl_p_i2c)){}

    // initialize I2C Master block w/ 400kbps data rate
    I2CMasterInitExpClk(i2c_base, SysCtlClockGet(), bFast);

    //clear I2C FIFOs
    //HWREG(I2C1_BASE + I2C_1_FIFOCTL) = 80008000;
}

void configureIMU(uint32_t i2c_base, uint8_t mpu6050_address, uint8_t hmc5883l_address, uint8_t ms5611_address)
{
    /* configure the MPU6050 (gyro/accelerometer)
	 * 2g = 16 384 counts/g
	 * 4g = 8 192 counts/g
	 * 8g = 4 096 counts/g
	 * 16g = 2 048 counts/g
	 *
	 */
	I2CSendValue(i2c_base, mpu6050_address,  0x6B, 0x00);                  		// exit sleep
	I2CSendValue(i2c_base, mpu6050_address,  0x19, 109);                   		// sample rate = 8kHz / 110 = 72.7Hz
	I2CSendValue(i2c_base, mpu6050_address,  0x1B, 0x18);                  		// gyro full scale = +/- 2000dps
	//I2CSendValue(i2c_base, mpu6050_address,  0x1B, 0x10);                  		// gyro full scale +/- 1000dps
	//I2CSendValue(i2c_base, mpu6050_address,  0x1C, 0x00);                  		// accelerometer full scale = +/- 2g
	//I2CSendValue(i2c_base, mpu6050_address,  0x1C, 0x08);                  		// accelerometer full scale = +/- 4g
	I2CSendValue(i2c_base, mpu6050_address,  0x1C, 0x10);                  		// accelerometer full scale = +/- 8g
	//I2CSendValue(i2c_base, mpu6050_address,  0x1C, 0x18);                  		// accelerometer full scale = +/- 16g

	// configure the MS5611 (barometer)
	I2CSendValueMS5611(i2c_base, ms5611_address, 0x1E);                    		// reset
	I2CSendValueMS5611(i2c_base, ms5611_address, 0x58);                    		// start conversion of the temperature sensor D2 = 4096
	I2CSendValueMS5611(i2c_base, ms5611_address, 0x48);                    		// start conversion of the pressure sensor D1 = 4096

	// configure the magnetometer
	I2CSendValue(i2c_base, mpu6050_address,  0x6A, 0x00);                    	// disable i2c master mode
	I2CSendValue(i2c_base, mpu6050_address,  0x37, 0x02);                    	// enable i2c master bypass mode
	I2CSendValue(i2c_base, hmc5883l_address, 0x00, 0x18);                    	// sample rate = 75Hz
	I2CSendValue(i2c_base, hmc5883l_address, 0x01, 0x60);                    	// full scale = +/- 2.5 Gauss
	I2CSendValue(i2c_base, hmc5883l_address, 0x02, 0x00);                    	// continuous measurement mode
	I2CSendValue(i2c_base, mpu6050_address,  0x37, 0x00);                    	// disable i2c master bypass mode
	I2CSendValue(i2c_base, mpu6050_address,  0x6A, 0x20);                    	// enable i2c master mode

	// configure the MPU6050 to automatically read the magnetometer
	I2CSendValue(i2c_base, mpu6050_address,  0x25, hmc5883l_address | 0x80); 	// slave 0 i2c address, read mode
	I2CSendValue(i2c_base, mpu6050_address,  0x26, 0x03);                    	// slave 0 register = 0x03 (x axis)
	I2CSendValue(i2c_base, mpu6050_address,  0x27, 6 | 0x80);                	// slave 0 transfer size = 6, enabled
	I2CSendValue(i2c_base, mpu6050_address,  0x67, 1);                       	// enable slave 0 delay
}
