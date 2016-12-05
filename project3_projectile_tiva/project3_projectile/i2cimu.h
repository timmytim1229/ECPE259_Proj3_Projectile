/*
 * i2cimu.h
 *
 *  Created on: Nov 17, 2016
 *      Author: krackletopwin
 */

#ifndef I2CIMU_H_
#define I2CIMU_H_

void I2CSendValue(uint32_t i2c_base, uint8_t slave_addr, uint8_t reg, uint8_t value);
void I2CReadValue(uint32_t i2c_base, uint8_t byte_count, uint8_t slave_addr, uint8_t reg_start, uint8_t *buffer);
void I2CSendValueMS5611(uint32_t i2c_base, uint8_t slave_addr, uint8_t cmd);

#endif /* I2CIMU_H_ */
