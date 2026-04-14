#include <stddef.h>
#include <stdio.h>
#include "API_uart.h"

#include "lcd1602.h"

extern lcd1602_error_t lcd1602_send_data(const lcd1602_data_t *lcd_data, bool read, bool write_to_data_reg, uint8_t payload);
extern lcd1602_error_t lcd1602_read_nibble(const lcd1602_data_t *lcd_data, bool is_data, uint8_t *nibble);
extern void lcd1602_delay_ms(uint32_t ms);

static lcd1602_error_t lcd1602_write_nibble(lcd1602_data_t *lcd_data, bool read, bool is_data, uint8_t value) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    uint8_t nibble = value & 0xF0; // Get the higher 4 bits
    return lcd1602_send_data(lcd_data, read, is_data, nibble);
}

static lcd1602_error_t lcd1602_write_bus(lcd1602_data_t *lcd_data, uint8_t value, bool is_data) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    if(IS_BIT_SET(lcd_data->disp_func, LCD1602_FUNCTION_SET_8BIT_IF)) {
        // 8-bit mode: the command is sent as is
        return lcd1602_send_data(lcd_data, false, is_data, value);
    } else {
        
        lcd1602_error_t err;
        // 4-bit mode: the command is sent in two parts (higher nibble first)
        err = lcd1602_write_nibble(lcd_data, false, is_data, value);
        if(err != OK) {
            return err;
        }
        // 4-bit mode: the command is sent in two parts (low nibble second)
        return lcd1602_write_nibble(lcd_data, false, is_data, (value << 4));
    }
}

static lcd1602_error_t lcd1602_write_command(lcd1602_data_t *lcd_data, uint8_t command) {
    return lcd1602_write_bus(lcd_data, command, false);
}

static lcd1602_error_t lcd1602_write_data(lcd1602_data_t *lcd_data, uint8_t data) {
    return lcd1602_write_bus(lcd_data, data, true);
}

static lcd1602_error_t lcd1602_wait_for_idle(lcd1602_data_t *lcd_data, uint8_t retries) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    for(uint8_t i = 0; i < retries; i++) {
        lcd1602_error_t err = lcd1602_is_busy(lcd_data);
        if(err == OK) {
            return OK;
        }
        if(err != BUSY) {
            return err; // propagate I2C errors
        }
    }
    return BUSY;
}

lcd1602_error_t lcd1602_init(lcd1602_data_t *lcd_data)
{
    if(lcd_data == NULL) {
        return -1;
    }   
    // this process assumes that the PSU for correctly operating the internal
    // reset circuit are not met, initialization by instructions becomes necessary.
    // because the device initialize in 8 bit mode, at first we only need to send
    // nibbles
    lcd1602_delay_ms(20);  // Wait for more than 15ms after VCC rises to 4.5V
    if(lcd1602_write_nibble(lcd_data, false, false, LCD1602_FUNCTION_SET | LCD1602_FUNCTION_SET_8BIT_IF) != OK) { // Function set: 4-bit mode
        uartSendString("Failed to set lcd function (1st)\r\n");
        return -1;
    }
    lcd1602_delay_ms(10);   // Wait for more than 4.1ms
    if(lcd1602_write_nibble(lcd_data, false, false, LCD1602_FUNCTION_SET | LCD1602_FUNCTION_SET_8BIT_IF) != OK) { // Function set: 4-bit mode
        uartSendString("Failed to set lcd function (2nd)\r\n");
        return -1;
    }
    lcd1602_delay_ms(1);   // Wait for more than 100µs
    if(lcd1602_write_nibble(lcd_data, false, false, LCD1602_FUNCTION_SET | LCD1602_FUNCTION_SET_8BIT_IF) != OK) { // Function set: 4-bit mode
        uartSendString("Failed to set lcd function (3rd)\r\n");
        return -1;
    }
    // Switch to 4-bit mode — busy flag is now readable after this nibble
    if(lcd1602_write_nibble(lcd_data, false, false, LCD1602_FUNCTION_SET) != OK) {
        uartSendString("Failed to set 4-bit mode\r\n");
        return -1;
    }
    // Function set: 4-bit mode, number of lines, font
    if(lcd1602_write_command(lcd_data, LCD1602_FUNCTION_SET | lcd_data->disp_func) != OK) {
        return -1;
    }
    if(lcd1602_wait_for_idle(lcd_data, 50) != OK) {
        return -1;
    }
    if(lcd1602_write_command(lcd_data, LCD1602_DISPLAY_CONTROL | lcd_data->disp_ctrl) != OK) {
        return -1;
    }
    if(lcd1602_wait_for_idle(lcd_data, 50) != OK) {
        return -1;
    }
    if(lcd1602_clear(lcd_data) != OK) {
        return -1;
    }
    if(lcd1602_wait_for_idle(lcd_data, 50) != OK) { // clear takes up to 1.52ms
        return -1;
    }
    if(lcd1602_write_command(lcd_data, LCD1602_ENTRY_MODE_SET | lcd_data->disp_mode) != OK) {
        return -1;
    }
    if(lcd1602_wait_for_idle(lcd_data, 50) != OK) {
        return -1;
    }
    if(lcd1602_return_home(lcd_data) != OK) {
        return -1;
    }
    if(lcd1602_wait_for_idle(lcd_data, 50) != OK) { // return home takes up to 1.52ms
        return -1;
    }
    return OK;
}

lcd1602_error_t lcd1602_is_busy(lcd1602_data_t* lcd_data)
{
    if(lcd_data == NULL) {
        return INVAL;
    }

    // Read the high nibble — D7 (busy flag) is in bit 7 of the returned byte.
    // In 4-bit mode a second nibble read is required to complete the HD44780
    // read cycle, even though we only care about the busy flag in the first.
    uint8_t high_nibble;
    lcd1602_error_t err = lcd1602_read_nibble(lcd_data, false, &high_nibble);
    if(err != OK) {
        return err;
    }

    if(!IS_BIT_SET(lcd_data->disp_func, LCD1602_FUNCTION_SET_8BIT_IF)) {
        uint8_t low_nibble;
        err = lcd1602_read_nibble(lcd_data, false, &low_nibble);
        if(err != OK) {
            return err;
        }
    }

    return IS_BIT_SET(high_nibble, LCD1602_BUSY_FLAG) ? BUSY : OK;
}

lcd1602_error_t lcd1602_clear(lcd1602_data_t *lcd_data) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    return lcd1602_write_command(lcd_data, LCD1602_CLEAR_DISPLAY);
}

lcd1602_error_t lcd1602_return_home(lcd1602_data_t *lcd_data) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    return lcd1602_write_command(lcd_data, LCD1602_RETURN_HOME);
}

lcd1602_error_t lcd1602_set_display_on(lcd1602_data_t *lcd_data, bool on) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    if(on) {
        lcd_data->disp_ctrl |= LCD1602_DISPLAY_ON;
    } else {
        lcd_data->disp_ctrl &= ~LCD1602_DISPLAY_ON;
    }
    return lcd1602_write_command(lcd_data, LCD1602_DISPLAY_CONTROL | lcd_data->disp_ctrl);
}

lcd1602_error_t lcd1602_set_cursor_on(lcd1602_data_t *lcd_data, bool on) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    if(on) {
        lcd_data->disp_ctrl |= LCD1602_CURSOR_ON;
    } else {
        lcd_data->disp_ctrl &= ~LCD1602_CURSOR_ON;
    }
    return lcd1602_write_command(lcd_data, LCD1602_DISPLAY_CONTROL | lcd_data->disp_ctrl);
}

lcd1602_error_t lcd1602_set_blink_on(lcd1602_data_t *lcd_data, bool on) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    if(on) {
        lcd_data->disp_ctrl |= LCD1602_BLINK_ON;
    } else {
        lcd_data->disp_ctrl &= ~LCD1602_BLINK_ON;
    }
    return lcd1602_write_command(lcd_data, LCD1602_DISPLAY_CONTROL | lcd_data->disp_ctrl);
}

lcd1602_error_t lcd1602_shift_display(lcd1602_data_t *lcd_data, bool right) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    uint8_t shift_command = LCD1602_CURSOR_AND_DISPLAY_SHIFT | LCD1602_CURSOR_SHIFT_DISPLAY;
    if(right) {
        shift_command |= LCD1602_CURSOR_SHIFT_RIGHT;
    }
    return lcd1602_write_command(lcd_data, shift_command);
}

lcd1602_error_t lcd1602_shift_cursor(lcd1602_data_t *lcd_data, bool right) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    uint8_t shift_command = LCD1602_CURSOR_AND_DISPLAY_SHIFT;
    if(right) {
        shift_command |= LCD1602_CURSOR_SHIFT_RIGHT;
    }
    return lcd1602_write_command(lcd_data, shift_command);
}

lcd1602_error_t lcd1602_set_interface_width(lcd1602_data_t *lcd_data, bool interface_8bit) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    if(interface_8bit) {
        lcd_data->disp_func |= LCD1602_FUNCTION_SET_8BIT_IF;
    } else {
        lcd_data->disp_func &= ~LCD1602_FUNCTION_SET_8BIT_IF;
    }
    return lcd1602_write_command(lcd_data, LCD1602_FUNCTION_SET | lcd_data->disp_func);
}

lcd1602_error_t lcd1602_set_display_lines(lcd1602_data_t *lcd_data, bool two_lines) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    if(two_lines) {
        lcd_data->disp_func |= LCD1602_FUNCTION_SET_2LINE;
    } else {
        lcd_data->disp_func &= ~LCD1602_FUNCTION_SET_2LINE;
    }
    return lcd1602_write_command(lcd_data, LCD1602_FUNCTION_SET | lcd_data->disp_func);
}

lcd1602_error_t lcd1602_set_font_size(lcd1602_data_t *lcd_data, bool font_5x10) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    if(font_5x10) {
        lcd_data->disp_func |= LCD1602_FUNCTION_SET_5X10;
    } else {
        lcd_data->disp_func &= ~LCD1602_FUNCTION_SET_5X10;
    }
    return lcd1602_write_command(lcd_data, LCD1602_FUNCTION_SET | lcd_data->disp_func);
}

lcd1602_error_t lcd1602_set_cursor(lcd1602_data_t *lcd_data, uint8_t col, uint8_t row) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    if(row >= lcd_data->num_lines) {
        return INVAL;
    }
    uint8_t address = col + lcd_data->row_offsets[row];
    return lcd1602_write_command(lcd_data, LCD1602_SET_DDRAM_ADDR | address);
}

lcd1602_error_t lcd1602_write_char(lcd1602_data_t *lcd_data, char c) {
    if(lcd_data == NULL) {
        return INVAL;
    }
    return lcd1602_write_data(lcd_data, (uint8_t)c);
}

lcd1602_error_t lcd1602_write_string(lcd1602_data_t *lcd_data, const char *str) {
    if(lcd_data == NULL || str == NULL) {
        return INVAL;
    }
    // it is assumed that the string is null-terminated, so we write characters
    // until we reach the null terminator
    while(*str != '\0') {
        lcd1602_error_t err = lcd1602_write_char(lcd_data, *str++);
        if(err != OK) {
            return err;
        }
    }
    return OK;
}