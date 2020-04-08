/*
 * i2c.h
 *
 *  Created on: April 8, 2020
 *      Author: Michael Bolt
 */

#ifndef I2C_H_
#define I2C_H_

#include <msp430.h>
#include <stdint.h>

#define I2C_TX_BUFFER_SIZE      128

void i2c_init(void);
uint16_t i2c_tx(uint8_t deviceAddress, const uint8_t *data, uint16_t count);


#endif  /* I2C_H_ */
