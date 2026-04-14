#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx_hal.h>

#include "API_uart.h"
#include "lcd1602.h"
#include "API_i2c.h"

#define BYTE_SIZE sizeof(uint8_t)

extern void lcd1602_delay_ms(uint32_t ms);
extern void lcd1602_delay_us(uint32_t us);

static uint8_t tx_buffer; /// one byte buffer to transmit data to LCD1602 display module via I2C communication

/**
 * @brief Send a command or data byte to the LCD1602 display module via I2C communication.
 *
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] read true if the operation is a read operation, false if it is a write operation
 * @param[in] write_to_data_reg true if the payload should be written to the data register,
 * false if it should be written to the command register
 * @param[in] payload The command or data byte to be sent to the LCD1602 display module
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 * @note This function is a placeholder and should be implemented to perform
 * actual I2C communication with the LCD1602 display module. It should also consider
 * the display settings in lcd_data to determine how to send the command or data
 * byte correctly. 
 */
lcd1602_error_t lcd1602_send_data(const lcd1602_data_t *lcd_data, bool read, bool write_to_data_reg, uint8_t payload) {
    /// check if all the information we need from lcd_data is valid
    if(lcd_data == NULL || lcd_data->port == NULL) {
        return -1;
    }
    i2c_port_t *port = (i2c_port_t *)lcd_data->port;
    if(port->i2c_handler == NULL) {
        return -1;
    }

    I2C_HandleTypeDef *i2c_handler = (I2C_HandleTypeDef *)port->i2c_handler;
    // Set the control bits for data/command and read/write
    tx_buffer = payload;
    /// | write_to_data_register | R/W  | EN OFF | EN ON | Backlight ON
    tx_buffer |= (write_to_data_reg * BIT(0)) | (read * BIT(1)) | BIT(2) | BIT(3) * lcd_data->backlight_on;

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(i2c_handler, port->address, &tx_buffer, BYTE_SIZE, HAL_MAX_DELAY);
    if(status != HAL_OK) {
        return WRITE_ERROR;
    }
    // EN bit should be held high for at least 450ns, so we can use a short delay here
    lcd1602_delay_us(1);
    tx_buffer &= ~BIT(2); // Clear the EN bit after transmission
    if(status != HAL_OK) {
        return WRITE_ERROR;
    }
    return OK;
}

/**
 * @brief Read a nibble from the LCD1602 via PCF8574T with a proper EN strobe.
 *
 * The PCF8574T only presents LCD output data on its pins while EN is held high.
 * This function drives EN high, performs the I2C read, then drives EN low, so
 * the returned byte contains valid LCD data in its upper nibble (D7-D4).
 *
 * @param[in]  lcd_data  Pointer to the lcd1602_data_t structure
 * @param[in]  is_data   true to assert RS (data register), false for command register
 * @param[out] nibble    Upper nibble of this byte holds D7-D4 from the LCD
 * @return OK on success, WRITE_ERROR or READ_ERROR on I2C failure
 */
lcd1602_error_t lcd1602_read_nibble(const lcd1602_data_t *lcd_data, bool is_data, uint8_t *nibble) {
    if(lcd_data == NULL || lcd_data->port == NULL || nibble == NULL) {
        return INVAL;
    }
    i2c_port_t *port = (i2c_port_t *)lcd_data->port;
    if(port->i2c_handler == NULL) {
        return INVAL;
    }
    I2C_HandleTypeDef *i2c_handler = (I2C_HandleTypeDef *)port->i2c_handler;

    // Drive data pins high (0xF0) so PCF8574 quasi-bidirectional I/Os act as inputs.
    // RW=1 (BIT1), RS as requested, backlight preserved.
    uint8_t bl  = lcd_data->backlight_on ? BIT(3) : 0;
    uint8_t rs  = is_data               ? BIT(0) : 0;
    uint8_t ctrl = 0xF0 | BIT(1) | bl | rs; // data pins high, RW=1, EN=0

    // Setup phase: EN=0
    uint8_t byte = ctrl & ~BIT(2);
    if(HAL_I2C_Master_Transmit(i2c_handler, port->address, &byte, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return WRITE_ERROR;
    }

    // Strobe high: EN=1 — LCD drives D7-D4 onto the bus
    byte = ctrl | BIT(2);
    if(HAL_I2C_Master_Transmit(i2c_handler, port->address, &byte, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return WRITE_ERROR;
    }

    // Read while EN is still high
    uint8_t data;
    if(HAL_I2C_Master_Receive(i2c_handler, port->address, &data, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return READ_ERROR;
    }
    *nibble = data & 0xF0; // only the upper nibble carries valid LCD data

    // Strobe low: EN=0 — latch the data
    byte = ctrl & ~BIT(2);
    if(HAL_I2C_Master_Transmit(i2c_handler, port->address, &byte, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return WRITE_ERROR;
    }

    return OK;
}