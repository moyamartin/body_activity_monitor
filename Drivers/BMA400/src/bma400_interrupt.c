#include <stddef.h>
#include <stm32f4xx_hal.h>

#include "bma400.h"

/**
 * @brief Initialize the interrupt GPIOs for the BMA400 sensor.
 *
 * @param[in] chip Pointer to the bma400_dev_t descriptor whose int1_config /
 *                 int2_config fields describe the MCU GPIOs connected to the
 *                 sensor's INT1 / INT2 pins.
 */
void bma400_init_interrupt_gpios(bma400_dev_t *chip) {
    if(chip == NULL) {
        return;
    }
    if(chip->int1_config == NULL && chip->int2_config == NULL) {
        // Interrupts not used, no need to initialize
        return;
    }

    if(chip->int1_config != NULL) {
        bma400_interrupt_t *int1_config = chip->int1_config;

        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = int1_config->pin;
        GPIO_InitStruct.Mode = int1_config->active_high ? GPIO_MODE_IT_RISING : GPIO_MODE_IT_FALLING;
        GPIO_InitStruct.Pull = int1_config->open_drain ? GPIO_PULLDOWN : GPIO_NOPULL;
        HAL_GPIO_Init((GPIO_TypeDef *)int1_config->port, &GPIO_InitStruct);
    }

    if(chip->int2_config != NULL) {
        bma400_interrupt_t *int2_config = chip->int2_config;

        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = int2_config->pin;
        GPIO_InitStruct.Mode = int2_config->active_high ? GPIO_MODE_IT_RISING : GPIO_MODE_IT_FALLING;
        GPIO_InitStruct.Pull = int2_config->open_drain ? GPIO_PULLDOWN : GPIO_NOPULL;
        HAL_GPIO_Init((GPIO_TypeDef *)int2_config->port, &GPIO_InitStruct);
    }
}
