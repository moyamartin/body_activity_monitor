/**
  * @file           : lcd1602.h
  * @brief          : Header for lcd1602.c file.
  *                   This file contains the driver for LCD1602 display module.
  */
#ifndef LCD1620_H_
#define LCD1620_H_

#include <stdbool.h>
#include <stdint.h>

#include "utils.h"


/* Commands definitions */
#define LCD1602_CLEAR_DISPLAY               0x01            ///< Screen clear, set AC  (address counter) to 0 cursor reposition
#define LCD1602_RETURN_HOME                 0x02            ///< Set AC to 0, return cursor to home position. DDRAM content is not changed.
#define LCD1602_ENTRY_MODE_SET              0x04            ///< Set cursor move direction and specify display shift
#define LCD1602_DISPLAY_CONTROL             0x08            ///< Set display ON/OFF and cursor ON/OFF
#define LCD1602_CURSOR_AND_DISPLAY_SHIFT    0x10            ///< Shift cursor position
#define LCD1602_FUNCTION_SET                0x20            ///< Set interface length, number of lines and font size
#define LCD1602_SET_CGRAM_ADDR              0x40            ///< Set CGRAM address
#define LCD1602_SET_DDRAM_ADDR              0x80            ///< Set DDRAM address

/* Entry Mode bits*/
#define LCD1602_ENTRY_MODE_SHIFT            BIT(0)          ///< 1: Display shift, 0: Cursor shift
#define LCD1602_ENTRY_MODE_INCREMENT        BIT(1)          ///< 1: Increment, 0: Decrement

/* Display Control bits */
#define LCD1602_DISPLAY_ON                  BIT(2)          ///< 1: Display ON, 0: Display OFF
#define LCD1602_CURSOR_ON                   BIT(1)          ///< 1: Cursor ON, 0: Cursor OFF
#define LCD1602_BLINK_ON                    BIT(0)          ///< 1: Blink ON, 0: Blink OFF

/* Cursor shift bits */
#define LCD1602_CURSOR_SHIFT_DISPLAY        BIT(3)          ///< 1: Display shift, 0: Cursor shift
#define LCD1602_CURSOR_SHIFT_RIGHT          BIT(2)          ///< 1: Right shift, 0: Left shift

/* Function Set bits */
#define LCD1602_FUNCTION_SET_8BIT_IF        BIT(4)          ///< 1: 8-bit interface, 0: 4-bit interface
#define LCD1602_FUNCTION_SET_2LINE          BIT(3)          ///< 1: 2 lines, 0: 1 line
#define LCD1602_FUNCTION_SET_5X10           BIT(2)          ///< 1: 5x10 style, 0: 5x8 style

/* Busy/AD Read CT */
#define LCD1602_BUSY_FLAG                  BIT(7)           ///< Busy flag is the highest bit of the data read from the display
#define LCD1602_ADDRESS_CGRAM_ADDRESS_MASK GENMASK(6, 0)    ///< Address counter value is the lower 7 bits of the data read from the display

/* device default values */
#define LCD1602_WIDTH 16
#define LCD1602_HEIGHT 2

typedef struct {
    void *port;             ///< Pointer to the communication port (e.g., I2C handle, GPIO pins. Usually defined by the manufacturer SDK)
    bool backlight_on;      ///< Backlight status (true for ON, false for OFF)
    uint8_t disp_func;      ///< Display function settings (interface length, number of lines, font size)
    uint8_t disp_ctrl;      ///< Display control settings (display ON/OFF, cursor ON/OFF, blink ON/OFF)
    uint8_t disp_mode;      ///< Entry mode settings (cursor move direction, display shift)
    uint8_t num_lines;      ///< Number of display lines (1 or 2)
    uint8_t row_offsets[4]; ///< Row offsets for addressing (depends on number of lines)
} lcd1602_data_t;

typedef enum {
    OK = 0,
    INVAL,
    WRITE_ERROR,
    READ_ERROR,
    BUSY,
} lcd1602_error_t;

/**
 * @brief Initialize the LCD1602 display module.
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure to be initialized.
 */
lcd1602_error_t lcd1602_init(lcd1602_data_t *lcd_data);

/**
 * @brief Clear the LCD1602 display.
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings/
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_clear(lcd1602_data_t *lcd_data);

/**
 * @brief Return the cursor to the home position (0,0) on the LCD1602 display.
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_return_home(lcd1602_data_t *lcd_data);

/**
 * @brief Turn display on/ff
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] on true to turn display on, false to turn display off
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_set_display_on(lcd1602_data_t *lcd_data, bool on);

/**
 * @brief Turn cursor on/off
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] on true to turn cursor on, false to turn cursor off
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_set_cursor_on(lcd1602_data_t *lcd_data, bool on);

/**
 * @brief Turn blink on/off
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] on true to turn blink on, false to turn blink off
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_set_blink_on(lcd1602_data_t *lcd_data, bool on);

/**
 * @brief Shift display left or right without affecting RAM
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] right true to shift right, false to shift left
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_shift_display(lcd1602_data_t *lcd_data, bool right);

/**
 * @brief Shift cursor left or right without affecting RAM
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] right true to shift right, false to shift left
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_shift_cursor(lcd1602_data_t *lcd_data, bool right);

/**
 * @brief Set data bus width (4-bit or 8-bit interface)
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] interface_8bit true if interface is 8-bit, false if 4-bit
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_set_interface_width(lcd1602_data_t *lcd_data, bool interface_8bit);

/**
 * @brief Set number of display lines (1 or 2)
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] two_line true if display has two lines, false if one line
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_set_display_lines(lcd1602_data_t *lcd_data, bool two_line);

/**
 * @brief Set font size (5x8 or 5x10 dots)
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] font_5x10 true if font is 5x10 dots, false if 5x8 dots
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_set_font_size(lcd1602_data_t *lcd_data, bool font_5x10);

/**
 * @brief Set the cursor to a specific position on the LCD1602 display.
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] col The column position (0-based index) to set the cursor to
 * @param[in] row The row position (0-based index) to set the cursor to
 * @return 0 on success, -1 on failure (e.g., invalid pointer, out of bounds)
 */
lcd1602_error_t lcd1602_set_cursor(lcd1602_data_t *lcd_data, uint8_t col, uint8_t row);

/**
 * @brief Write a single character to the LCD1602 display at the current cursor position.
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] c The character to be written to the LCD1602 display
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_write_char(lcd1602_data_t *lcd_data, char c);

/**
 * @brief Write a null-terminated string to the LCD1602 display starting at the current cursor position.
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[in] str The null-terminated string to be written to the LCD1602 display
 * @return 0 on success, -1 on failure (e.g., invalid pointer
 */
lcd1602_error_t lcd1602_write_string(lcd1602_data_t *lcd_data, const char *str);

/**
 * @brief Check if the LCD1602 display is busy processing a command or data.
 * @param[in] lcd_data Pointer to the lcd1602_data_t structure containing display settings
 * @param[out] busy Pointer to a boolean variable where the busy status will be stored
 * @return 0 on success, -1 on failure (e.g., invalid pointer)
 */
lcd1602_error_t lcd1602_is_busy(lcd1602_data_t *lcd_data);


#endif /* LCD1602_H_ */