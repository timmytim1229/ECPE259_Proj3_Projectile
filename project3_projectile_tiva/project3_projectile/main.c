//*****************************************************************************
//
// main.c
//
// Written by David De La Vega for ECPE/CIVL 259 - Sensor Networks
//
// This program uses the TI Tiva tm4c123gh6pm.
//
// Much code was used from Farrellf's Balancing Robot Firmware.
// The GY-86 Sensor Package was also used.
// https://github.com/farrellf/Balancing_Robot_Firmware
//
//*****************************************************************************

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
#include "mucConfig.h"
#include "readimu.h"

// globals for current state
float accel_x_g;
float accel_y_g;
float accel_z_g;
float mpu_temp_c;
float gyro_x_rad;
float gyro_y_rad;
float gyro_z_rad;
float magn_x_gs;
float magn_y_gs;
float magn_z_gs;
float pressure_float;
float baro_temp_float;

FATFS FatFs;
FIL LogFile;
FRESULT Res;

// i2c device addresses
#define MPU6050_ADDRESS  0b1101000
#define HMC5883L_ADDRESS 0b0011110
#define MS5611_ADDRESS   0b1110111

void uartTest()
{
	//UARTCharPut(UART1_BASE, 's');
	UARTCharPut(UART0_BASE, 'a');
}

int main(void)
{
    volatile uint32_t ui32Loop;

    SysCtlClockSet( SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_4 );

    // configure and enable all needed busses
    // UART0 -> USB Port
    // UART1 -> XBEE Radio
    // UART2 -> GPS Module
    UARTConfig(SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_GPIOA, GPIO_PA0_U0RX, GPIO_PIN_0, GPIO_PA1_U0TX, GPIO_PIN_1, GPIO_PORTA_BASE, UART0_BASE, 9600);
    UARTConfig(SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_GPIOC, GPIO_PC4_U1RX, GPIO_PIN_4, GPIO_PC5_U1TX, GPIO_PIN_5, GPIO_PORTC_BASE, UART1_BASE, 57600);
    UARTConfig(SYSCTL_PERIPH_UART2, SYSCTL_PERIPH_GPIOD, GPIO_PD6_U2RX, GPIO_PIN_6, GPIO_PD7_U2TX, GPIO_PIN_7, GPIO_PORTD_BASE, UART2_BASE, 9600);
    I2CConfig(SYSCTL_PERIPH_I2C1, GPIO_PA6_I2C1SCL, GPIO_PIN_6, GPIO_PA7_I2C1SDA, GPIO_PIN_7, GPIO_PORTA_BASE, I2C1_BASE, true);
    SPIinit();
    I2CMasterEnable(I2C1_BASE);
    configureIMU(I2C1_BASE, MPU6050_ADDRESS, HMC5883L_ADDRESS, MS5611_ADDRESS);
    UARTEnable(UART0_BASE);
    UARTEnable(UART1_BASE);
    UARTEnable(UART2_BASE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // read GPS data from UART2 for initial distance
    //readGPS(UART2_BASE);

    //while(UARTCharsAvail(UART1_BASE)){
    while(1){ //wait for signal from program to exit
    	// toggle GPIO pin for frequency measurement


    	//mpu6050test(UART0_BASE, I2C1_BASE, MPU6050_ADDRESS);
    	readGPS(UART2_BASE, UART1_BASE);
    	//UARTCharPut(UART1_BASE, '%');
    	//GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 1);
    	//readMPU6050(UART1_BASE, I2C1_BASE, MPU6050_ADDRESS);
    	//readMS5611(UART1_BASE, I2C1_BASE, MS5611_ADDRESS);

    	//uartTest();
    	// read whoami: 0x75, should get 68 from it
    	//GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0);

    	//
    }
    // read GPS data from UART2 for final distance
    //readGPS(UART2_BASE);



}
