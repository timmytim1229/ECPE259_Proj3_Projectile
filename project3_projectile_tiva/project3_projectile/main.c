//*****************************************************************************
//
// main.c
//
// Written by David De La Vega for ECPE/CIVL 259 - Sensor Networks
//
// This program uses the TI Tiva tm4c123gh6pm.
//
//*****************************************************************************

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_i2c.h"
#include <stdint.h>
#include <stdbool.h>
#include <driverlib/sysctl.h>
#include <driverlib/adc.h>
#include <inc/hw_memmap.h>			// for ADC & UART0
#include <stdio.h>
#include <driverlib/interrupt.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/uart.h>
#include <driverlib/i2c.h>



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

// i2c device addresses
#define MPU6050_ADDRESS  0x68
#define HMC5883L_ADDRESS 0x1E
#define MS5611_ADDRESS   0x77


/* configure UART0 and UART1
 *
 */
void UARTStartup()
{
    // enable the UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    // Configure the GPIO pins as UART and output pins
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinConfigure(GPIO_PC4_U1RX);
    GPIOPinConfigure(GPIO_PC5_U1TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, (GPIO_PIN_0 | GPIO_PIN_1));
    GPIOPinTypeUART(GPIO_PORTC_BASE, (GPIO_PIN_4 | GPIO_PIN_5));

    // Configure the UART: 8-bit communication, 1 stop bit, no parity
    // change baudrate to 57,600 later
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600,
         UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
         UART_CONFIG_PAR_NONE);

    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
         UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
         UART_CONFIG_PAR_NONE);
}

void I2CStartup()
{
	// Enable the I2C module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    // reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

	// configure pins for I2C1 line
	GPIOPinConfigure(GPIO_PA6_I2C1SCL);
	GPIOPinConfigure(GPIO_PA7_I2C1SDA);
	GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C1)){}

    // initialize I2C Master block w/ 400kbps data rate
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), true);

    //clear I2C FIFOs
    HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
}

// 1. enacts protocol for GY-86 Sensor package
// write address to read from/write to
// write or read data
void I2CSendValue(uint32_t i2c_base, uint8_t slave_addr, uint8_t reg, uint8_t value)
{
	I2CMasterSlaveAddrSet(i2c_base, slave_addr, false);				// write to slave_addr

	I2CMasterDataPut(i2c_base, reg);								// write register address
	I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_START);
	while(I2CMasterBusy(i2c_base));									// while not busy

	I2CMasterDataPut(i2c_base, value);								// write value to register
	I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(I2CMasterBusy(i2c_base));									// while not busy
}

void I2CReadValue(uint32_t i2c_base, uint8_t slave_addr, uint8_t reg)
{
	I2CMasterSlaveAddrSet(i2c_base, slave_addr, false);				// write to slave_addr
	I2CMasterDataPut(i2c_base, reg);								//specify what register to read from

    I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_SEND_START);	//send control byte and register address byte to slave device
    while(I2CMasterBusy(I2C0_BASE));								//wait for MCU to finish transaction


    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, true);				// read from slave_addr
    I2CMasterControl(I2C0_BASE, );


    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
    while(I2CMasterBusy(I2C0_BASE));								//wait for MCU to finish transaction





	I2CMasterDataGet(i2c_base, )



}

void configureIMU()
{
    // configure the MPU6050 (gyro/accelerometer)
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x6B, 0x00);                  	// exit sleep
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x19, 109);                   	// sample rate = 8kHz / 110 = 72.7Hz
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x1B, 0x18);                  	// gyro full scale = +/- 2000dps
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x1C, 0x08);                  	// accelerometer full scale = +/- 4g

	// configure the MS5611 (barometer)
	I2CSendValue(I2C1_BASE, MS5611_ADDRESS, 0x1E, 0x00);                    	// reset
	I2CSendValue(I2C1_BASE, MS5611_ADDRESS, 0x48, 0x00);                    	// start conversion of the pressure sensor

	// configure the magnetometer
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x6A, 0x00);                    	// disable i2c master mode
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x37, 0x02);                    	// enable i2c master bypass mode
	I2CSendValue(I2C1_BASE, HMC5883L_ADDRESS, 0x00, 0x18);                    	// sample rate = 75Hz
	I2CSendValue(I2C1_BASE, HMC5883L_ADDRESS, 0x01, 0x60);                    	// full scale = +/- 2.5 Gauss
	I2CSendValue(I2C1_BASE, HMC5883L_ADDRESS, 0x02, 0x00);                    	// continuous measurement mode
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x37, 0x00);                    	// disable i2c master bypass mode
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x6A, 0x20);                    	// enable i2c master mode

	// configure the MPU6050 to automatically read the magnetometer
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x25, HMC5883L_ADDRESS | 0x80); 	// slave 0 i2c address, read mode
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x26, 0x03);                    	// slave 0 register = 0x03 (x axis)
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x27, 6 | 0x80);                	// slave 0 transfer size = 6, enabled
	I2CSendValue(I2C1_BASE, MPU6050_ADDRESS,  0x67, 1);                       	// enable slave 0 delay
}


void readIMU()
{
	uint8_t rx_buffer[20];


}


void readGPS()
{
    int num_gps_char = 0;
    int gps_return = 0;
    char gps_data [256];
    char frame_char1, frame_char2, frame_char3;

    while(gps_return == 0){

		frame_char1 = UARTCharGet(UART1_BASE);
		frame_char2 = UARTCharGet(UART1_BASE);
		frame_char3 = UARTCharGet(UART1_BASE);

		// Read lat/long from GPS
		// Search for GPS start characters
		// $GP --> GLL <---
		if(frame_char1 == 'G' && frame_char2 == 'G' && frame_char3 == 'A') {

			// Stuff the "GLL" through the UART
			// G
			gps_data[num_gps_char] = frame_char1;
			UARTCharPut(UART0_BASE, gps_data[num_gps_char]);
			num_gps_char++;
			// L
			gps_data[num_gps_char] = frame_char2;
			UARTCharPut(UART0_BASE, gps_data[num_gps_char]);
			num_gps_char++;
			// L
			gps_data[num_gps_char] = frame_char3;
			UARTCharPut(UART0_BASE, gps_data[num_gps_char]);
			num_gps_char++;

			// read until end of the frame (terminated with an \r)
			while(gps_data[num_gps_char] != '\r') {
				gps_data[num_gps_char] = UARTCharGet(UART1_BASE);
				UARTCharPut(UART0_BASE, gps_data[num_gps_char]);
				num_gps_char++;
			}
			UARTCharPut(UART0_BASE,'\0'); // send null character
			gps_return = 1; //return from function
		}
    }
}

void gyroAccelWrite()
{

}


int main(void)
{
    volatile uint32_t ui32Loop;

    int i;

    // enable the clock
    SysCtlClockSet( SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_4 );

    UARTStartup();		//intialize UART
    I2CStartup();		//initalize I2C
    configureIMU();			//setup IMU


    // Enable UART0_BASE
    UARTEnable(UART0_BASE);
    // Enable UART1_BASE
    UARTEnable(UART1_BASE);


    //while(UARTCharsAvail(UART1_BASE)){
    while(1){
    	// read GPS data from UART1
    	// readGPS();

    	// read whoami: 0x75, should get 68 from it

    }

    // send GPS data through USB serial
    //for(i = 0; i < 256; i++){
    //	UARTCharPut(UART0_BASE, gps_data[i]);
    //}
    //UARTCharPut(UART0_BASE,'\0'); // send null character

	// toggle GPIO pin for frequency measurement
	//GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
	//SysCtlDelay(5000); 	// Delay for 5000 clock cycles
	//GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, ~GPIO_PIN_4);
}
