#include "API_delay.h"
#include "stm32f4xx_hal.h"

void delay_init(delay_t *delay, tick_t duration) {
    if(delay == NULL ) { return; }
    delay->startTime = 0;
    delay->duration = duration;
    delay->running = false;
}

bool_t delay_read(delay_t *delay) {
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

void delay_stop(delay_t *delay) {
  delay->running = false;
}

void delay_write(delay_t *delay, tick_t duration) {
  if(delay == NULL ) { return; }
  delay->duration = duration;
}

bool_t delay_is_running(const delay_t *delay) {
	if(delay == NULL) { return false; }
	return delay->running;
}
