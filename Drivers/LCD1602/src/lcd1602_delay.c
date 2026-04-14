#include <stdint.h>

#include <stm32f4xx_hal.h>

extern TIM_HandleTypeDef htim1;

/**
 * @brief Perform delay of the specified number of milliseconds.
 * @param[in] ms The number of milliseconds to delay
 * @note This function is a placeholder and should be implemented to perform
 * an actual delay using a timer or a blocking loop, depending on the platform.
 */
void lcd1602_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

void lcd1602_delay_us(uint32_t us) {
    // For microsecond delay, we can use a timer-based approach for better accuracy
    // Assuming TIM1 is configured for microsecond timing
    HAL_TIM_Base_Start(&htim1); // Start the timer
    htim1.Instance->CNT = 0; // Reset the timer count
    while (htim1.Instance->CNT < us); // Wait until the desired time has elapsed
}