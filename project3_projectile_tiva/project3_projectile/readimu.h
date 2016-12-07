/*
 * readimu.h
 *
 *  Created on: Nov 28, 2016
 *      Author: krackletopwin
 */

#ifndef READIMU_H_
#define READIMU_H_

void readMPU6050(uint32_t uart_base, uint32_t i2c_base, uint8_t mpu6050_address);
void readMS5611(uint32_t uart_base, uint32_t i2c_base, uint8_t ms5611_address);
void readGPS(uint32_t uart_get_base, uint32_t uart_put_base);
void mpu6050test(uint32_t uart_base, uint32_t i2c_base, uint8_t mpu6050_address);

#endif /* SD_READIMU_H_ */
