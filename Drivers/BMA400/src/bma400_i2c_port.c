#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx_hal.h>

#include "bma400.h"
#include "API_i2c.h"
#include "API_uart.h"

/**
 * @brief Send data to the BMA400 module via I2C communication.
 *
 * @param[in] chip     Pointer to the BMA400 device descriptor
 * @param[in] reg_addr Register address to write
 * @param[in] size     Number of bytes in @p payload
 * @param[in] payload  Buffer containing the bytes to write
 * @return BMA400_OK on success, BMA400_INVAL on invalid pointer, BMA400_WRITE_ERROR on I2C failure
 */
bma400_error_t bma400_send_data(const bma400_dev_t *chip, uint8_t reg_addr, size_t size, uint8_t *payload) {
    if(chip == NULL || chip->intf_ptr == NULL || payload == NULL || size == 0) {
        return BMA400_INVAL;
    }
    i2c_port_t *port = (i2c_port_t *)chip->intf_ptr;
    if(port->i2c_handler == NULL) {
        return BMA400_INVAL;
    }

    I2C_HandleTypeDef *i2c_handler = (I2C_HandleTypeDef *)port->i2c_handler;
    /**
     * BMA400 I2C does not autoincrement reg_addr on writes, so walk through
     * registers one byte at a time.
     */
    for(size_t i = 0; i < size; i++) {
        HAL_StatusTypeDef status = HAL_I2C_Mem_Write(i2c_handler, port->address, reg_addr + i, I2C_MEMADD_SIZE_8BIT, payload + i, 1, HAL_MAX_DELAY);
        if(status != HAL_OK) {
            return BMA400_WRITE_ERROR;
        }
    }
    return BMA400_OK;
}

/**
 * @brief Read data from the BMA400 via I2C.
 *
 * @param[in]  chip     Pointer to the BMA400 device descriptor
 * @param[in]  reg_addr Register address to read
 * @param[in]  size     Number of bytes to read
 * @param[out] data     Destination buffer
 * @return BMA400_OK on success, BMA400_INVAL on invalid pointer, BMA400_READ_ERROR on I2C failure
 */
bma400_error_t bma400_read_data(const bma400_dev_t *chip, uint8_t reg_addr, size_t size, uint8_t *data) {
    if(chip == NULL || chip->intf_ptr == NULL || data == NULL || size == 0) {
        return BMA400_INVAL;
    }
    i2c_port_t *port = (i2c_port_t *)chip->intf_ptr;
    if(port->i2c_handler == NULL) {
        return BMA400_INVAL;
    }

    I2C_HandleTypeDef *i2c_handler = (I2C_HandleTypeDef *)port->i2c_handler;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c_handler, port->address, reg_addr, I2C_MEMADD_SIZE_8BIT, data, size, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        return BMA400_READ_ERROR;
    }
    return BMA400_OK;
}