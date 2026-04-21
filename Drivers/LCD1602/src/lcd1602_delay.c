/**
 * @file   lcd1602_delay.c
 * @brief  STM32 implementation of the LCD1602 delay hooks.
 *
 * The HD44780 controller requires precise waits between nibbles during the
 * 4-bit init sequence and after each command (~37 us for typical ops, up to
 * 1.52 ms for clear/home). Two granularities are provided:
 *   - millisecond delays use HAL_Delay() (SysTick-based);
 *   - microsecond delays use TIM1, configured by CubeMX as a 1 MHz timebase.
 */
#include <stdint.h>

#include <stm32f4xx_hal.h>

/* TIM1 handle owned by CubeMX in main.c; shared with the BMA400 delay. */
extern TIM_HandleTypeDef htim1;

/**
 * @brief  Block the CPU for @p ms milliseconds.
 * @param[in] ms Number of milliseconds to wait.
 */
void lcd1602_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

/**
 * @brief  Block the CPU for @p us microseconds using TIM1 as a free-running
 *         1 MHz counter. Used for the short EN strobes and inter-nibble waits
 *         required by the HD44780 state machine.
 * @param[in] us Number of microseconds to wait.
 */
void lcd1602_delay_us(uint32_t us) {
    HAL_TIM_Base_Start(&htim1);
    htim1.Instance->CNT = 0;
    /* Busy-wait: CNT increments once per microsecond. */
    while (htim1.Instance->CNT < us);
}