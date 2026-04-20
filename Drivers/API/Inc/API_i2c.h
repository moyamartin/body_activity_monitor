/**
  * @file           : API_i2c.h
  * @brief          : Header for the generic I2C port abstraction.
  *                   This file defines the common type used by drivers
  *                   that communicate with devices over an I2C bus,
  *                   decoupling them from the underlying MCU SDK.
  */
#ifndef API_I2C_PORT_H_
#define API_I2C_PORT_H_

#include <stdint.h>

/**
 * @brief I2C port descriptor
 *
 * Generic abstraction that binds an opaque bus handle (provided by the
 * manufacturer SDK, e.g. an STM32 HAL I2C_HandleTypeDef) with the 7-bit
 * address of the target device. Drivers operate on this type so they stay
 * independent of the underlying HAL.
 */
typedef struct {
    void *i2c_handler;  ///< Pointer to the I2C handle for communication
    uint8_t address;    ///< I2C address of the device to talk to
} i2c_port_t;

#endif /* API_I2C_PORT_H_ */