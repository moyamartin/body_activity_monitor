#include "API_delay.h"
#include "stm32f4xx_hal.h"

void delayInit(delay_t *delay, tick_t duration) {
    if(delay == NULL ) { return; }
    delay->startTime = 0;
    delay->duration = duration;
    delay->running = false;
}

bool_t delayRead(delay_t *delay) {
  if(delay == NULL ) { return false; }
  if(delay->running) {
    /// Check if the delay has elapsed
    if(HAL_GetTick() - delay->startTime >= delay->duration) {
      delay->running = false;
      return true;
    }
  } else {
    /// if not running, start delay
    delay->startTime = HAL_GetTick();
    delay->running = true;
  }
  return false;
}

void delayWrite(delay_t *delay, tick_t duration) {
  if(delay == NULL ) { return; }
  delay->duration = duration;
}

bool_t delayIsRunning(const delay_t *delay) {
	if(delay == NULL) { return false; }
	return delay->running;
}
