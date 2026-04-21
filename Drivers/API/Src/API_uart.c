/**
 * @file   API_uart.c
 * @brief  Thin wrapper around the STM32 HAL UART driver used for the
 *         serial console (USART2, routed through the ST-Link VCP).
 *
 * The UART handle is kept module-local so the rest of the application does
 * not need to know which peripheral is behind the console. All transfers are
 * blocking; a single static tx_buffer is reused by the formatted print path.
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "API_uart.h"
#include "stm32f4xx_hal.h"


static UART_HandleTypeDef huart;
static char tx_buffer[MAX_TX_SIZE];


uart_error_t uart_init()
{
  huart.Instance = USART2;
  huart.Init.BaudRate = 115200;
  huart.Init.WordLength = UART_WORDLENGTH_8B;
  huart.Init.StopBits = UART_STOPBITS_1;
  huart.Init.Parity = UART_PARITY_NONE;
  huart.Init.Mode = UART_MODE_TX_RX;
  huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart.Init.OverSampling = UART_OVERSAMPLING_16;
  if(HAL_UART_Init(&huart) != HAL_OK) {
	  return UART_INIT_FAILED;
  }
  snprintf(tx_buffer,
		   sizeof(tx_buffer)/sizeof(tx_buffer[0]),
		   "baudrate: %lu, word length: %u, stop bits: %u\r\n", // parity: %s, mode: %lu, hw flow ctrl: %lu, oversampling: %lu\r\n",
		   huart.Init.BaudRate, (huart.Init.WordLength == 0) ? 8 : 9,
		   (huart.Init.StopBits == 0) ? 1 : 2); // (huart.Init.Parity == UART_PARITY_NONE) "none" ? "even", huart.Init.Mode,
		   //huart.Init.HwFlowCtl, huart.Init.OverSampling);
  uart_send_string(tx_buffer);
  return UART_OK;
}

uart_error_t uart_send_string(char *pstring)
{
	if(pstring == NULL) {
		return UART_INVAL;
	}
	size_t size = strlen(pstring);
	if(size > MAX_TX_SIZE || size < MIN_TX_SIZE) {
		return UART_INVAL;
	}
	if(HAL_UART_Transmit(&huart, (uint8_t*)pstring, (uint16_t)size, HAL_MAX_DELAY) != HAL_OK) {
		return UART_TRANSMIT_ERROR;
	}
	return UART_OK;
}

uart_error_t uart_send_string_size(char *pstring, uint16_t size)
{
	if(pstring == NULL) {
		return UART_INVAL;
	}
	if(size > MAX_TX_SIZE || size < MIN_TX_SIZE) {
		return UART_INVAL;
	}
	if(HAL_UART_Transmit(&huart, (uint8_t*) pstring, size, HAL_MAX_DELAY) != HAL_OK) {
		return UART_TRANSMIT_ERROR;
	}
	return UART_OK;
}

uart_error_t uart_send_formatted_string(const char *fmt, ...)
{
	if(fmt == NULL) {
		return UART_INVAL;
	}
	va_list args;
	va_start(args, fmt);
	int written = vsnprintf(tx_buffer, sizeof(tx_buffer), fmt, args);
	va_end(args);
	if(written <= 0) {
		return UART_INVAL;
	}
	/* vsnprintf returns the size it WOULD have written; clamp to the buffer
	 * so an oversized format string does not produce a spurious length. */
	size_t size = ((size_t)written >= sizeof(tx_buffer))
					? sizeof(tx_buffer) - 1
					: (size_t)written;
	return uart_send_string_size(tx_buffer, size);
}

uart_error_t uart_receive_string_size(char *pstring, uint16_t size)
{
	if(pstring == NULL) {
		return UART_INVAL;
	}
	if(size > MAX_RX_SIZE || size < MIN_RX_SIZE) {
		return UART_INVAL;
	}
	if(HAL_UART_Receive(&huart, (uint8_t *)pstring, size, HAL_MAX_DELAY) != HAL_OK) {
		return UART_RECEIVE_ERROR;
	}
	return UART_OK;
}
