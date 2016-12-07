/*
 * readimu.c
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
#include "i2cimu.h"


void readMPU6050(uint32_t uart_base, uint32_t i2c_base, uint8_t mpu6050_address)
{
	uint8_t rx_buffer[20];
	char send_array[16];
	int i = 0;

	// obtain data from MPU650 (20 bytes, register 0x3B)
	// bytes 0-5 = accel data
	// bytes 6-7 = temperature data
	// bytes 8-13 = gyro data
	// bytes 14-19 = magno data
	I2CReadValue(i2c_base, 20, mpu6050_address, 0x3B, rx_buffer);

	// send label to start data packet
	UARTCharPut(uart_base, 'S');
	UARTCharPut(uart_base, 'A');
	for(i = 0; i <= 13; i++){
    UARTCharPut(uart_base, rx_buffer[i]);

    //after every two bytes, send a delimiter
//    if(i%2 != 0){
//      UARTCharPut(uart_base, '\n');
//    }

    // send temp data label
    if(i == 5){
      UARTCharPut(uart_base, 'T');
      //UARTCharPut(uart_base, '\n');
    }

    // send temp data label
    if(i == 7){
      UARTCharPut(uart_base, 'G');
      //UARTCharPut(uart_base, '\n');
    }
  }
//  UARTCharPut(uart_base, 'D');
//  UARTCharPut(uart_base, 'D');
  UARTCharPut(uart_base, '\n');
}

void readMS5611(uint32_t uart_base, uint32_t i2c_base, uint8_t ms5611_address)
{
	uint8_t rx_buffer[3];
	int i = 0;

	// write
	// read MPU6050
	I2CReadValue(i2c_base, 3, ms5611_address, 0x00, rx_buffer);

	for(i = 0; i < 3; i++)
		UARTCharPut(uart_base, rx_buffer[i]);
}

//void readGPS(uint32_t uart_get_base, uint32_t uart_put_base)
//{
//    int num_gps_char = 0;
//    int gps_return = 0;
//    char gps_data [128];
//    char frame_char1, frame_char2, frame_char3;
//
//    while(gps_return == 0){
//		// Read lat/long from GPS
//		// Search for GPS start characters
//		// $GP --> GLL <---
//
////		//$
////		gps_data[num_gps_char] = UARTCharGet(uart_get_base);
////		UARTCharPut(uart_put_base, gps_data[num_gps_char]);
////		num_gps_char++;
////		//G
////		gps_data[num_gps_char] = UARTCharGet(uart_get_base);
////		UARTCharPut(uart_put_base, gps_data[num_gps_char]);
////		num_gps_char++;
////		//P
////		gps_data[num_gps_char] = UARTCharGet(uart_get_base);
////		UARTCharPut(uart_put_base, gps_data[num_gps_char]);
////		num_gps_char++;
//
//		frame_char1 = UARTCharGet(uart_get_base);
////		frame_char2 = UARTCharGet(uart_get_base);
////		frame_char3 = UARTCharGet(uart_get_base);
//		//if(frame_char1 == 'G' && frame_char2 == 'G' && frame_char3 == 'A') {
//		if(frame_char1 == '$') {
//			// Stuff the "GLL" through the UART
//			// G
//			gps_data[num_gps_char] = frame_char1;
//			UARTCharPut(uart_put_base, gps_data[num_gps_char]);
//			num_gps_char++;
////			// L
////			gps_data[num_gps_char] = frame_char2;
////			UARTCharPut(uart_put_base, gps_data[num_gps_char]);
////			num_gps_char++;
////			// L
////			gps_data[num_gps_char] = frame_char3;
////			UARTCharPut(uart_put_base, gps_data[num_gps_char]);
////			num_gps_char++;
//			// read until end of the frame (terminated with an \n)
//			while(gps_data[num_gps_char] != '\n') {
//				gps_data[num_gps_char] = UARTCharGet(uart_get_base);
//				UARTCharPut(uart_put_base, gps_data[num_gps_char]);
//				num_gps_char++;
//			}
////			gps_data[num_gps_char] = UARTCharGet(uart_get_base);
////			UARTCharPut(uart_put_base, gps_data[num_gps_char]);
//			//UARTCharPut(uart_put_base,'\n'); // send null character
//			//gps_data[num_gps_char] = '\0';
//
//			//gps_return = 1; //return from function
//		}
//    }
//}


void readGPS(uint32_t uart_get_base, uint32_t uart_put_base)
{
    int num_gps_char = 0;
    int gps_return = 0;
    char gps_data;
    //char gps_data [128];
    char frame_char1, frame_char2, frame_char3;

    while(gps_return == 0){
		// Read lat/long from GPS
		// Search for GPS start characters
		// $GP --> GLL <---

		frame_char1 = UARTCharGet(uart_get_base);
		if(frame_char1 == '$') {

			//stuff $ through UART
			gps_data = frame_char1;
			UARTCharPut(uart_put_base, gps_data);
			num_gps_char++;

			// read until end of the frame (terminated with an \n)
			while(gps_data != '\n') {
				gps_data = UARTCharGet(uart_get_base);
				UARTCharPut(uart_put_base, gps_data);
				num_gps_char++;
			}
			//UARTCharPut(uart_put_base,'\n'); // send null character
			//return from function
			gps_return = 1;
		}
    }
}

void mpu6050test(uint32_t uart_base, uint32_t i2c_base, uint8_t mpu6050_address)
{
	uint8_t test_val = 0;
	I2CReadValue(i2c_base, 1, mpu6050_address, 0x75, &test_val);		// read MPU6050
	UARTCharPut(uart_base, '%');
	UARTCharPut(uart_base, test_val);
}
