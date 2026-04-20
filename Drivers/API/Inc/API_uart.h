/**
  * @file           : API_uart.h
  * @brief          : Header for API_uart.c file.
  *                   This file contains the common defines and prototypes
  *                   of the UART driver wrapper used by the application.
  */
#ifndef API_UART_H_
#define API_UART_H_

#include "API_delay.h"

#define MAX_TX_SIZE 256     ///< Maximum number of bytes accepted by a single transmit call
#define MIN_TX_SIZE 1       ///< Minimum number of bytes accepted by a single transmit call
#define MAX_RX_SIZE 256     ///< Maximum number of bytes accepted by a single receive call
#define MIN_RX_SIZE 1       ///< Minimum number of bytes accepted by a single receive call

/**
 * @brief Return codes produced by the UART API
 */
typedef enum {
	UART_OK,                ///< Operation completed successfully
	UART_INVAL,             ///< Invalid argument (NULL pointer or size out of range)
	UART_INIT_FAILED,       ///< Underlying HAL UART initialization failed
	UART_TRANSMIT_ERROR,    ///< Error while transmitting bytes over the bus
	UART_RECEIVE_ERROR,     ///< Error while receiving bytes over the bus
} uart_error_t;

/**
 * @brief Initialize the UART instance
 * @return UART_OK on success, UART_INIT_FAILED otherwise
 * @note On success it prints over UART a message with the active configuration
 * parameters (baud rate, word length, parity and stop bits).
 */
uart_error_t uart_init();

/**
 * @brief Send a null-terminated string over UART
 * @param[in] pstring pointer to the null-terminated string to transmit
 * @return UART_OK on success, UART_INVAL if @p pstring is NULL or its length
 * is out of the [MIN_TX_SIZE, MAX_TX_SIZE] range, UART_TRANSMIT_ERROR if the
 * underlying transfer failed.
 */
uart_error_t uart_send_string(char *pstring);

/**
 * @brief Send a string of a specific size over UART
 * @param[in] pString pointer to the buffer to transmit
 * @param[in] size number of bytes to transmit; must be within
 * [MIN_TX_SIZE, MAX_TX_SIZE]
 * @return UART_OK on success, UART_INVAL if @p pString is NULL or @p size is
 * out of range, UART_TRANSMIT_ERROR if the underlying transfer failed.
 */
uart_error_t uart_send_string_size(char *pString, uint16_t size);

/**
 * @brief Receive a string of a specific size over UART
 * @param[out] pstring pointer to the buffer where received data will be stored
 * @param[in] size number of bytes to read; must be within
 * [MIN_RX_SIZE, MAX_RX_SIZE]
 * @return UART_OK on success, UART_INVAL if @p pstring is NULL or @p size is
 * out of range, UART_RECEIVE_ERROR if the underlying transfer failed.
 */
uart_error_t uart_receive_string_size(char *pstring, uint16_t size);

/**
 * @brief Send a printf-style formatted string over UART
 * @param[in] fmt format string (printf-compatible)
 * @param[in] ... arguments matching the format specifiers
 * @return UART_OK on success, UART_INVAL if @p fmt is NULL,
 * UART_TRANSMIT_ERROR if the underlying transfer failed.
 * @note Output is truncated to MAX_TX_SIZE - 1 characters. The function
 * attribute `format(printf, 1, 2)` enables compile-time format checking.
 */
uart_error_t uart_send_formatted_string(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

#endif /* API_UART_H_ */
