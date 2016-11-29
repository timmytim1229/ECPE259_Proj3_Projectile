/*
 * mucConfig.h
 *
 *  Created on: Nov 28, 2016
 *      Author: krackletopwin
 */

#ifndef MUCCONFIG_H_
#define MUCCONFIG_H_

void UARTConfig(uint32_t sysctl_p_uart, uint32_t sysctl_p_gpio, uint32_t rxpin, uint32_t rxpin_num, uint32_t txpin, uint32_t txpin_num, uint32_t gpio_port_base, uint32_t uart_base, uint32_t baud_rate);
void I2CConfig(uint32_t sysctl_p_i2c, uint32_t gpio_scl, uint32_t scl_pin_num, uint32_t gpio_sda, uint32_t sda_pin_num, uint32_t gpio_port_base, uint32_t i2c_base, bool bFast);
void configureIMU(uint32_t i2c_base, uint8_t mpu6050_address, uint8_t hmc5883l_address, uint8_t ms5611_address);

#endif /* MUCCONFIG_H_ */
