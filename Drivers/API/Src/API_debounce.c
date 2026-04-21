/**
 * @file   API_debounce.c
 * @brief  Software debouncer for the onboard user button B1 (PC13).
 *
 * Implements a 4-state FSM (UP -> FALLING -> DOWN -> RISING -> UP) driven by
 * a non-blocking 40 ms delay. Edges are validated only if the GPIO still
 * reads the expected level after the delay expires, filtering mechanical
 * bouncing without blocking the main loop. A press event is latched in
 * @ref keyPressed and consumed once by read_key().
 */
#include "API_debounce.h"
#include "stm32f4xx_hal.h"

#define ANTIBOUNCE_DELAY 40U      ///< debounce window in ms; empirically enough for B1
#define B1_Pin GPIO_PIN_13        ///< Nucleo user button (B1)
#define B1_GPIO_Port GPIOC

static debounceState_t debounceFSM;
static delay_t delayDebounce;
static bool_t keyPressed;         ///< pending press event, cleared by read_key()
static bool_t isInitialized = false;

void debounce_init()
{
	delay_init(&delayDebounce, ANTIBOUNCE_DELAY);
	debounceFSM = BUTTON_UP;
	isInitialized = true;
}

void debounce_update()
{
	if(isInitialized == false) { return; }
	/* B1 is active-low on the Nucleo: pressed => RESET, released => SET. */
	switch(debounceFSM) {
	case BUTTON_UP:
		/* Idle: wait for a falling edge (press started). */
		if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
			debounceFSM = BUTTON_FALLING;
		}
		break;
	case BUTTON_FALLING:
		/* Confirm the press still holds after the debounce window. */
		if(delay_read(&delayDebounce) && HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
			debounceFSM = BUTTON_DOWN;
			keyPressed = true;
		}
		break;
	case BUTTON_DOWN:
		/* Wait for the release to start. */
		if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
			debounceFSM = BUTTON_RISING;
		}
		break;
	case BUTTON_RISING:
		/* Confirm the release still holds after the debounce window. */
		if(delay_read(&delayDebounce) && HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
			debounceFSM = BUTTON_UP;
		}
		break;
	default:
		break;
	}
}

bool_t read_key() {
	if(isInitialized == false) { return false; }
	bool_t temp_value = keyPressed;
	keyPressed = false;
	return temp_value;
}
