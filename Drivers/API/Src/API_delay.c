/**
 * @file   API_delay.c
 * @brief  Non-blocking delay primitives backed by the HAL SysTick counter.
 *
 * A delay is an opaque timestamp plus a duration: delay_read() starts the
 * timer the first time it is called with running=false, and returns true
 * (auto-stopping the delay) once HAL_GetTick() has advanced by at least the
 * configured duration. The unsigned subtraction wraps correctly at the
 * 32-bit tick rollover (~49.7 days), so no special handling is needed.
 */
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
    /* Unsigned subtraction => safe across the SysTick wraparound. */
    if(HAL_GetTick() - delay->startTime >= delay->duration) {
      delay->running = false;
      return true;
    }
  } else {
    /* First call after init or after elapse: (re)arm the timer. */
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
