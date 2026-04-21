/**
 * @file   lcd1602_i2c_port.c
 * @brief  LCD1602 interface port for the PCF8574 I2C backpack on STM32 HAL.
 *
 * The PCF8574 is an 8-bit quasi-bidirectional I/O expander wired to the
 * HD44780 controller as follows (typical backpack layout):
 *
 *   P0 = RS, P1 = RW, P2 = EN, P3 = backlight, P4..P7 = D4..D7
 *
 * This port implements the two low-level hooks required by the portable
 * LCD1602 driver (lcd1602.c):
 *   - lcd1602_send_data(): writes one nibble while driving the EN strobe
 *     (high-then-low) so the HD44780 latches D7..D4 on the falling edge.
 *   - lcd1602_read_nibble(): reads the busy flag / address counter by
 *     tri-stating the data pins (writing 1s), asserting RW=1 and strobing EN
 *     high during the I2C read transaction.
 *
 * Every I2C write emits a single byte through HAL_I2C_Master_Transmit(); the
 * module-local @ref tx_buffer is reused for all transmissions to keep stack
 * usage flat. Microsecond delays between EN transitions are honoured via
 * lcd1602_delay_us() so the HD44780 timing requirements are met.
 */
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

    uint8_t ctrl = (write_to_data_reg ? BIT(0) : 0)
                 | (read               ? BIT(1) : 0)
                 | (lcd_data->backlight_on ? BIT(3) : 0);
    uint8_t data_bits = payload & 0xF0;

    tx_buffer = data_bits | ctrl;
    if(HAL_I2C_Master_Transmit(i2c_handler, port->address, &tx_buffer, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return LCD1602_WRITE_ERROR;
    }

    // EN=1
    tx_buffer |= BIT(2);
    if(HAL_I2C_Master_Transmit(i2c_handler, port->address, &tx_buffer, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return LCD1602_WRITE_ERROR;
    }
    lcd1602_delay_us(1);

    // EN=0  data is latched on this falling edge
    tx_buffer &= ~BIT(2);
    if(HAL_I2C_Master_Transmit(i2c_handler, port->address, &tx_buffer, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return LCD1602_WRITE_ERROR;
    }
    lcd1602_delay_us(50); // most HD44780 commands execute in ~37 µs

    return LCD1602_OK;
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
        return LCD1602_INVAL;
    }
    i2c_port_t *port = (i2c_port_t *)lcd_data->port;
    if(port->i2c_handler == NULL) {
        return LCD1602_INVAL;
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
        return LCD1602_WRITE_ERROR;
    }

    // Strobe high: EN=1 -- LCD drives D7-D4 onto the bus
    byte = ctrl | BIT(2);
    if(HAL_I2C_Master_Transmit(i2c_handler, port->address, &byte, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return LCD1602_WRITE_ERROR;
    }

    // Read while EN is still high
    uint8_t data;
    if(HAL_I2C_Master_Receive(i2c_handler, port->address, &data, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return LCD1602_READ_ERROR;
    }
    *nibble = data & 0xF0; // only the upper nibble carries valid LCD data

    // Strobe low: EN=0 -- latch the data
    byte = ctrl & ~BIT(2);
    if(HAL_I2C_Master_Transmit(i2c_handler, port->address, &byte, BYTE_SIZE, HAL_MAX_DELAY) != HAL_OK) {
        return LCD1602_WRITE_ERROR;
    }

    return LCD1602_OK;
}