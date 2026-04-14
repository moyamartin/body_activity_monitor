#ifndef API_I2C_PORT_H_
#define API_I2C_PORT_H_

#include <stdint.h>

typedef struct {
    void *i2c_handler;  ///< Pointer to the I2C handle for communication
    uint8_t address;    ///< I2C address of the LCD1602 display module
} i2c_port_t;

#endif