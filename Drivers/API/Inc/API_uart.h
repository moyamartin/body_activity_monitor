#ifndef API_UART_H_
#define API_UART_H_

#include "API_delay.h"

#define MAX_TX_SIZE 256
#define MIN_TX_SIZE 1
#define MAX_RX_SIZE 256
#define MIN_RX_SIZE 1

/**
 * @brief initialize UART instance
 * @return true if everything went OK, false if something wrong happened
 * @note it will print over UART a msg with it's configuration parameters
 */
bool_t uartInit();

/**
 * @brief sends a string over uart
 * @param[in] pString: pointer to array of chars
 */
void uartSendString(char *pstring);

/**
 * @brief send a string over uart with a specific size
 * @param[in] pString: pointer to array of chars
 * @param[in] size: size of string. Must be between 1 and 256
 */
void uartSendStringSize(uint8_t *pString, uint16_t size);

/**
 * @brief receive a string over uart
 * @param[out] pstring: pointer to buffer where the data will be stored
 * @param[in] size: amount of bytes to read
 */
bool_t uartReceiveStringSize(uint8_t *pstring, uint16_t size);

#endif /* API_UART_H_ */
