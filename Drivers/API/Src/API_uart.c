#include <string.h>
#include <stdio.h>

#include "API_uart.h"
#include "stm32f4xx_hal.h"


static UART_HandleTypeDef huart;
static char tx_buffer[MAX_TX_SIZE];


bool_t uartInit()
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
	  return false;
  }
  snprintf(tx_buffer,
		   sizeof(tx_buffer)/sizeof(tx_buffer[0]),
		   "baudrate: %lu, word length: %u, stop bits: %u\r\n", // parity: %s, mode: %lu, hw flow ctrl: %lu, oversampling: %lu\r\n",
		   huart.Init.BaudRate, (huart.Init.WordLength == 0) ? 8 : 9,
		   (huart.Init.StopBits == 0) ? 1 : 2); // (huart.Init.Parity == UART_PARITY_NONE) "none" ? "even", huart.Init.Mode,
		   //huart.Init.HwFlowCtl, huart.Init.OverSampling);
  uartSendString(tx_buffer);
  return true;
}

void uartSendString(char *pstring)
{
	if(pstring == NULL) {
		return;
	}
	size_t size = strlen(pstring);
	if(size > MAX_TX_SIZE || size < MIN_TX_SIZE) {
		return;
	}
	HAL_UART_Transmit(&huart, (uint8_t*)pstring, (uint16_t)size, HAL_MAX_DELAY);
}

void uartSendStringSize(uint8_t *pstring, uint16_t size)
{
	if(pstring == NULL) {
		return;
	}
	if(size > MAX_TX_SIZE || size < MIN_TX_SIZE) {
		return;
	}
	HAL_UART_Transmit(&huart, pstring, size, HAL_MAX_DELAY);
}

bool uartReceiveStringSize(uint8_t *pstring, uint16_t size)
{
	if(pstring == NULL) {
		return false;
	}
	if(size > MAX_RX_SIZE || size < MIN_RX_SIZE) {
		return false;
	}
	return HAL_UART_Receive(&huart, pstring, size, HAL_MAX_DELAY) != HAL_OK;
}
