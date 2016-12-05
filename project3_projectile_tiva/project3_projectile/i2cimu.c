/*
 * i2cimu.c
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

// 1. enacts protocol for GY-86 Sensor package
// write address to read from/write to
// write or read data
void I2CSendValue(uint32_t i2c_base, uint8_t slave_addr, uint8_t reg, uint8_t value)
{
	I2CMasterSlaveAddrSet(i2c_base, slave_addr, false);					// write to slave_addr

	I2CMasterDataPut(i2c_base, reg);									// write register address
	I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(i2c_base));										// wait for MCU to finish transaction

	I2CMasterDataPut(i2c_base, value);									// write value to register
	I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(i2c_base));										// while not busy
}

void I2CSendValueMS5611(uint32_t i2c_base, uint8_t slave_addr, uint8_t cmd)
{
	I2CMasterSlaveAddrSet(i2c_base, slave_addr, false);					// write to slave_addr

	I2CMasterDataPut(i2c_base, cmd);									// write register address
	I2CMasterControl(i2c_base, I2C_MASTER_CMD_SINGLE_SEND);
	while(I2CMasterBusy(i2c_base));										// wait for MCU to finish transaction
}

void I2CReadValue(uint32_t i2c_base, uint8_t byte_count, uint8_t slave_addr, uint8_t reg_start, uint8_t *buffer)
{
	int bcm1;
	uint32_t temp_val = 0;

	I2CMasterSlaveAddrSet(i2c_base, slave_addr, false);						// write to slave_addr
	I2CMasterDataPut(i2c_base, reg_start);									// specify what register to read from
    I2CMasterControl(i2c_base, I2C_MASTER_CMD_SINGLE_SEND);					// send control byte and register address byte to slave device
    while(I2CMasterBusy(i2c_base));											// wait for MCU to finish transaction

    I2CMasterSlaveAddrSet(i2c_base, slave_addr, true);						// read from slave_addr

    if(byte_count > 1) {
		I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_START);
		while(I2CMasterBusy(i2c_base));										// wait for MCU to finish transaction
		temp_val = I2CMasterDataGet(i2c_base);
		*buffer++ = (uint8_t)temp_val;

		bcm1 = byte_count - 1;

		while(bcm1-- > 1){
			I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
			while(I2CMasterBusy(i2c_base));									// wait for MCU to finish transaction
			temp_val = I2CMasterDataGet(i2c_base);
			*buffer++ = (uint8_t)temp_val;
		}

		I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
		while(I2CMasterBusy(i2c_base));										// wait for MCU to finish transaction
		temp_val = I2CMasterDataGet(i2c_base);
		*buffer++ = (uint8_t)temp_val;

    } else { //byte_count == 1
    	I2CMasterControl(i2c_base, I2C_MASTER_CMD_SINGLE_RECEIVE);
		while(I2CMasterBusy(i2c_base));										// wait for MCU to finish transaction
		temp_val = I2CMasterDataGet(i2c_base);
		*buffer = (uint8_t)temp_val;
    }
}
