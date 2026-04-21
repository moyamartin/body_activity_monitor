/**
 * @file   bma400_delay.c
 * @brief  STM32 implementation of the BMA400 delay hooks.
 *
 * Two granularities are provided:
 *   - millisecond delays use HAL_Delay() (SysTick-based);
 *   - microsecond delays use TIM1, which CubeMX configures with a 1 MHz tick
 *     (prescaler = 83 over a 84 MHz APB2 clock).
 */
#include <stdint.h>

#include <stm32f4xx_hal.h>

/* TIM1 handle owned by CubeMX in main.c; borrowed here as a 1 MHz timebase. */
extern TIM_HandleTypeDef htim1;

/**
 * @brief  Block the CPU for @p ms milliseconds.
 * @param[in] ms Number of milliseconds to wait.
 */
void bma400_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

/**
 * @brief  Block the CPU for @p us microseconds using TIM1 as a free-running
 *         1 MHz counter. Intended for short device settling delays (~100 us
 *         or less); not designed for long waits.
 * @param[in] us Number of microseconds to wait.
 */
void bma400_delay_us(uint32_t us) {
    HAL_TIM_Base_Start(&htim1);
    htim1.Instance->CNT = 0;
    /* Busy-wait: CNT increments once per microsecond. */
    while (htim1.Instance->CNT < us);
}