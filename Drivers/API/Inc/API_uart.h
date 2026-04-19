#ifndef API_UART_H_
#define API_UART_H_

#include "API_delay.h"

#define MAX_TX_SIZE 256
#define MIN_TX_SIZE 1
#define MAX_RX_SIZE 256
#define MIN_RX_SIZE 1

typedef enum {
	UART_OK,
	UART_INVAL,
	UART_INIT_FAILED,
	UART_TRANSMIT_ERROR,
	UART_RECEIVE_ERROR,
} uart_error_t;

/**
 * @brief initialize UART instance
 * @return true if everything went OK, false if something wrong happened
 * @note it will print over UART a msg with it's configuration parameters
 */
uart_error_t uart_init();

/**
 * @brief sends a string over uart
 * @param[in] pString: pointer to array of chars
 */
uart_error_t uart_send_string(char *pstring);

/**
 * @brief send a string over uart with a specific size
 * @param[in] pString: pointer to array of chars
 * @param[in] size: size of string. Must be between 1 and 256
 */
uart_error_t uart_send_string_size(char *pString, uint16_t size);

/**
 * @brief receive a string over uart
 * @param[out] pstring: pointer to buffer where the data will be stored
 * @param[in] size: amount of bytes to read
 */
uart_error_t uart_receive_string_size(char *pstring, uint16_t size);

/**
 * @brief sends a printf-style formatted string over uart
 * @param[in] fmt: format string
 * @param[in] ...: arguments matching the format specifiers
 * @note output is truncated to MAX_TX_SIZE - 1 characters
 */
uart_error_t uart_send_formatted_string(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

#endif /* API_UART_H_ */
