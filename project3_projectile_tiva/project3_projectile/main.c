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
#include <stdint.h>
#include <stdbool.h>
#include <driverlib/sysctl.h>
#include <driverlib/adc.h>
#include <inc/hw_memmap.h>			// for ADC & UART0
#include <stdio.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/uart.h>
#include <driverlib/i2c.h>
#include <stdarg.h>
#include <string.h>
#include "SD/diskio.h"
#include "SD/ff.h"
#include "SD/microSD.h"
#include "SD/spi_SD.h"

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
/*
#define MPU6050_ADDRESS  0x68
#define HMC5883L_ADDRESS 0x1E
#define MS5611_ADDRESS   0x77
*/


/* configure UART0 and UART1
 *
 */

void writeSDchar(char data)
{
	f_mount(&FatFs, "", 0); //Mount volume
	f_open(&LogFile, "test.log", FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&LogFile, f_size(&LogFile));

	//f_putc(data, &LogFile);
	f_printf(&LogFile, "%c", data);
	f_close(&LogFile);
}

void writeSD(uint8_t data)
{
	f_mount(&FatFs, "", 0); //Mount volume
	f_open(&LogFile, "test.log", FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&LogFile, f_size(&LogFile));

	//f_putc(data, &LogFile);
	f_printf(&LogFile, "%x", data);
	f_close(&LogFile);
}

void writeSDMPU(char *data)
{
	f_mount(&FatFs, "", 0); //Mount volume
	f_open(&LogFile, "test.log", FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&LogFile, f_size(&LogFile));

	//uint8_t written;
	//f_putc(data, &LogFile);

	f_puts(data, &LogFile);
	f_close(&LogFile);
}


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

    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 57600,
         UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
         UART_CONFIG_PAR_NONE);
}

void I2CStartup()
{
	// Enable the I2C module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    // reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);

	// configure pins for I2C1 line
	GPIOPinConfigure(GPIO_PA6_I2C1SCL);
	GPIOPinConfigure(GPIO_PA7_I2C1SDA);
	GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C1)){}

    // initialize I2C Master block w/ 400kbps data rate
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), true);

    //clear I2C FIFOs
    //HWREG(I2C1_BASE + I2C_1_FIFOCTL) = 80008000;
}

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

void I2CReadValue(uint32_t i2c_base, uint8_t byte_count, uint8_t slave_addr, uint8_t reg_start, uint8_t *buffer)
{
	int bcm1;
	uint32_t temp_val = 0;

	I2CMasterSlaveAddrSet(i2c_base, slave_addr, false);						//write to slave_addr
	I2CMasterDataPut(i2c_base, reg_start);									//specify what register to read from
    I2CMasterControl(i2c_base, I2C_MASTER_CMD_SINGLE_SEND);					//send control byte and register address byte to slave device
    while(I2CMasterBusy(i2c_base));											//wait for MCU to finish transaction

    I2CMasterSlaveAddrSet(i2c_base, slave_addr, true);						// read from slave_addr

    if(byte_count > 1) {
		I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_START);
		while(I2CMasterBusy(i2c_base));										//wait for MCU to finish transaction
		temp_val = I2CMasterDataGet(i2c_base);
		*buffer++ = (uint8_t)temp_val;

		bcm1 = byte_count - 1;

		while(bcm1-- > 1){
			I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
			while(I2CMasterBusy(i2c_base));									//wait for MCU to finish transaction
			temp_val = I2CMasterDataGet(i2c_base);
			*buffer++ = (uint8_t)temp_val;
		}

		I2CMasterControl(i2c_base, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
		while(I2CMasterBusy(i2c_base));										//wait for MCU to finish transaction
		temp_val = I2CMasterDataGet(i2c_base);
		*buffer++ = (uint8_t)temp_val;

    } else { //byte_count == 1
    	I2CMasterControl(i2c_base, I2C_MASTER_CMD_SINGLE_RECEIVE);
		while(I2CMasterBusy(i2c_base));										//wait for MCU to finish transaction
		temp_val = I2CMasterDataGet(i2c_base);
		*buffer = (uint8_t)temp_val;
    }
}

void configureIMU(uint32_t i2c_base)
{
    // configure the MPU6050 (gyro/accelerometer)
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x6B, 0x00);                  	// exit sleep
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x19, 109);                   	// sample rate = 8kHz / 110 = 72.7Hz
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x1B, 0x18);                  	// gyro full scale = +/- 2000dps
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x1C, 0x08);                  	// accelerometer full scale = +/- 4g

	// configure the MS5611 (barometer)
	I2CSendValue(i2c_base, MS5611_ADDRESS, 0x1E, 0x00);                    	// reset
	I2CSendValue(i2c_base, MS5611_ADDRESS, 0x48, 0x00);                    	// start conversion of the pressure sensor

	// configure the magnetometer
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x6A, 0x00);                    	// disable i2c master mode
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x37, 0x02);                    	// enable i2c master bypass mode
	I2CSendValue(i2c_base, HMC5883L_ADDRESS, 0x00, 0x18);                    	// sample rate = 75Hz
	I2CSendValue(i2c_base, HMC5883L_ADDRESS, 0x01, 0x60);                    	// full scale = +/- 2.5 Gauss
	I2CSendValue(i2c_base, HMC5883L_ADDRESS, 0x02, 0x00);                    	// continuous measurement mode
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x37, 0x00);                    	// disable i2c master bypass mode
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x6A, 0x20);                    	// enable i2c master mode

	// configure the MPU6050 to automatically read the magnetometer
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x25, HMC5883L_ADDRESS | 0x80); 	// slave 0 i2c address, read mode
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x26, 0x03);                    	// slave 0 register = 0x03 (x axis)
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x27, 6 | 0x80);                	// slave 0 transfer size = 6, enabled
	I2CSendValue(i2c_base, MPU6050_ADDRESS,  0x67, 1);                       	// enable slave 0 delay
}


void readMPU6050(uint32_t uart_base, uint32_t i2c_base)
{
	uint8_t rx_buffer[20];
	char send_array[16];
	int i = 0;

	// obtain data from MPU650 (20 bytes, register 0x3B)
  // bytes 0-5 = accel data
  // bytes 6-7 = temperature data
  // bytes 8-13 = gyro data
  // bytes 14-19 = magno data
	I2CReadValue(i2c_base, 20, MPU6050_ADDRESS, 0x3B, rx_buffer);

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
  UARTCharPut(uart_base, '\n');
}


void readMS5611()
{
	uint8_t rx_buffer[3];
	int i = 0;

	// read MPU6050
	I2CReadValue(I2C1_BASE, 3, MS5611_ADDRESS, 0x00, rx_buffer);

	f_mount(&FatFs, "", 0); //Mount volume
	f_open(&LogFile, "test.log", FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&LogFile, f_size(&LogFile));

	f_printf(&LogFile, "%c", 'B');
	f_printf(&LogFile, "%c", ':');
	for(i = 0; i < 3; i++)
		f_printf(&LogFile, "%x", rx_buffer[i]);
	f_printf(&LogFile, "%c", '\n');

	f_close(&LogFile);
}


void readGPS()
{
    int num_gps_char = 0;
    int gps_return = 0;
    char gps_data [128];
    char frame_char1, frame_char2, frame_char3;

	f_mount(&FatFs, "", 0); //Mount volume
	f_open(&LogFile, "test.log", FA_WRITE | FA_OPEN_ALWAYS);
	f_lseek(&LogFile, f_size(&LogFile));

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
			//UARTCharPut(UART0_BASE, gps_data[num_gps_char]);
			//f_printf(&LogFile, "%c", gps_data[num_gps_char]);
			num_gps_char++;
			// L
			gps_data[num_gps_char] = frame_char2;
			//UARTCharPut(UART0_BASE, gps_data[num_gps_char]);
			//f_printf(&LogFile, "%c", gps_data[num_gps_char]);
			num_gps_char++;
			// L
			gps_data[num_gps_char] = frame_char3;
			//UARTCharPut(UART0_BASE, gps_data[num_gps_char]);
			//f_printf(&LogFile, "%c", gps_data[num_gps_char]);
			num_gps_char++;

			// read until end of the frame (terminated with an \n)
			while(gps_data[num_gps_char] != '\n') {
				gps_data[num_gps_char] = UARTCharGet(UART1_BASE);
				//UARTCharPut(UART0_BASE, gps_data[num_gps_char]);
				//f_printf(&LogFile, "%c", gps_data[num_gps_char]);
				num_gps_char++;
			}
			//UARTCharPut(UART0_BASE,'\n');
			//f_printf(&LogFile, "%c", '\n');
			//UARTCharPut(UART0_BASE,'\n'); // send null character
			gps_data[num_gps_char] = '\0';


			f_printf(&LogFile, "%s", gps_data);
			f_close(&LogFile);
			gps_return = 1; //return from function
		}
    }
}

void mpu6050test()
{
	uint8_t test_val = 0;
	I2CReadValue(I2C1_BASE, 1, MPU6050_ADDRESS, 0x75, &test_val);		// read MPU6050
	UARTCharPut(UART0_BASE, '%');
	UARTCharPut(UART0_BASE, test_val);
}

void uartTest()
{
	UARTCharPut(UART1_BASE, 's');
	UARTCharPut(UART0_BASE, 'a');
}


int main(void)
{
    volatile uint32_t ui32Loop;

    // enable the clock
    SysCtlClockSet( SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_SYSDIV_4 );

    UARTStartup();					//intialize UART
    I2CStartup();						//initalize I2C
    SPIinit();							//initialize SPI

    I2CMasterEnable(I2C1_BASE);    // Enable I2C

    configureIMU(I2C1_BASE);	//setup IMU


    // Enable UART0_BASE
    UARTEnable(UART0_BASE);
    // Enable UART1_BASE
    UARTEnable(UART1_BASE);


    //while(UARTCharsAvail(UART1_BASE)){
    while(1){
    	// read GPS data from UART1
     	//readGPS();
    	//mpu6050test();
    	readMPU6050(UART1_BASE, I2C1_BASE);
    	//readMS5611();

    	//uartTest();
    	// read whoami: 0x75, should get 68 from it

    }
	// toggle GPIO pin for frequency measurement
	//GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_PIN_4);
	//SysCtlDelay(5000); 	// Delay for 5000 clock cycles
	//GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4, ~GPIO_PIN_4);
}
