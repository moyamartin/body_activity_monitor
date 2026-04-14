#ifndef LCD1602_I2C_PORT_H_
#define LCD1602_I2C_PORT_H_

#include <stdint.h>

typedef struct {
    void *i2c_handler;  ///< Pointer to the I2C handle for communication
    uint8_t address;    ///< I2C address of the LCD1602 display module
} lcd1602_i2c_port_t;

#endif