#include "API_debounce.h"
#include "stm32f4xx_hal.h"

#define ANTIBOUNCE_DELAY 40U
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

static debounceState_t debounceFSM;
static delay_t delayDebounce;
static bool_t keyPressed;
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
	switch(debounceFSM) {
	case BUTTON_UP:
		if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
			debounceFSM = BUTTON_FALLING;
		}
		break;
	case BUTTON_FALLING:
		if(delay_read(&delayDebounce) && HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) {
			debounceFSM = BUTTON_DOWN;
			keyPressed = true;
		}
		break;
	case BUTTON_DOWN:
		if(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
			debounceFSM = BUTTON_RISING;
		}
		break;
	case BUTTON_RISING:
		if(delay_read(&delayDebounce) && HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET) {
			debounceFSM = BUTTON_UP;
		}
		break;
	default:
		// error
		break;
	}
}

bool_t read_key() {
	if(isInitialized == false) { return false; }
	bool_t temp_value = keyPressed;
	keyPressed = false;
	return temp_value;
}
