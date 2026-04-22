/**
 * @file   bma400_i2c_port.c
 * @brief  BMA400 interface port for I2C on STM32 HAL.
 *
 * This file provides the two platform hooks consumed by the portable BMA400
 * driver (bma400.c):
 *   - bma400_send_data(): register write. A particularity of the BMA400 I2C
 *     interface is that it does NOT auto-increment the register address on
 *     writes, so multi-byte writes are unrolled into one HAL_I2C_Mem_Write()
 *     call per byte.
 *   - bma400_read_data(): register read. Reads DO auto-increment, so a
 *     single HAL_I2C_Mem_Read() covers an arbitrary-length block.
 *
 * The device address (including the R/W bit, i.e. shifted left by 1 for the
 * STM32 HAL) and the underlying I2C_HandleTypeDef are resolved indirectly
 * through @ref bma400_dev_t::intf_ptr, which the application sets to an
 * @ref i2c_port_t instance. This keeps the driver itself free of HAL
 * includes so it can be dropped into a different MCU by swapping this port.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx_hal.h>

#include "bma400.h"
#include "API_i2c.h"
#include "API_uart.h"

/**
 * Maximum payload for a single bma400_send_data() transmission. The longest
 * register burst currently issued by the driver is 11 bytes (Gen1/Gen2
 * configuration block), so 16 leaves headroom without wasting stack.
 */
#define BMA400_MAX_WRITE_PAYLOAD 16

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
    if(size > BMA400_MAX_WRITE_PAYLOAD) {
        return BMA400_INVAL;
    }
    i2c_port_t *port = (i2c_port_t *)chip->intf_ptr;
    if(port->i2c_handler == NULL) {
        return BMA400_INVAL;
    }

    I2C_HandleTypeDef *i2c_handler = (I2C_HandleTypeDef *)port->i2c_handler;
    /**
     * BMA400 I2C does not autoincrement reg_addr on writes, so each register
     * has to carry its own address. Pack the (addr,value) pairs into one
     * contiguous frame and emit it in a single transmission to avoid
     * re-addressing the slave on every byte:
     *
     *   frame = [reg_addr, payload[0], reg_addr+1, payload[1], ...]
     */
    uint8_t frame[2 * BMA400_MAX_WRITE_PAYLOAD];
    for(size_t i = 0; i < size; i++) {
        frame[2 * i]     = (uint8_t)(reg_addr + i);
        frame[2 * i + 1] = payload[i];
    }

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(i2c_handler, port->address, frame, (uint16_t)(2 * size), HAL_MAX_DELAY);
    if(status != HAL_OK) {
        return BMA400_WRITE_ERROR;
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