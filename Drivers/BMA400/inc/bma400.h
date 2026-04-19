#ifndef BMA400_H_
#define BMA400_H_

#include <stddef.h>
#include <stdint.h>

#include "bma400_def.h"

/**
 * @brief Port-level register write (implemented by the selected interface port).
 */
bma400_error_t bma400_send_data(const bma400_dev_t *chip, uint8_t reg_addr,
                                size_t size, uint8_t *payload);

/**
 * @brief Port-level register read (implemented by the selected interface port).
 */
bma400_error_t bma400_read_data(const bma400_dev_t *chip, uint8_t reg_addr,
                                size_t size, uint8_t *payload);

/**
 * @brief Read (and cache) the BMA400 chip-id register.
 */
bma400_error_t bma400_chip_id(bma400_dev_t *chip, uint8_t *chip_id);

/**
 * @brief Issue a command through the CMD register, waiting for cmd_rdy.
 */
bma400_error_t bma400_send_cmd(bma400_dev_t *chip, uint8_t cmd);

/**
 * @brief Soft-reset the device (CMD 0xB6).
 */
bma400_error_t bma400_soft_reset(bma400_dev_t *chip);

/**
 * @brief Clear the step counter (CMD 0xB1).
 */
bma400_error_t bma400_clear_steps(bma400_dev_t *chip);

/**
 * @brief Initialise the device: verify chip-id, soft-reset, configure interrupts.
 */
bma400_error_t bma400_init(bma400_dev_t *chip);

/**
 * @brief Apply the interrupt pin configuration stored in chip->int1_config / int2_config.
 */
bma400_error_t bma400_configure_interrupts(bma400_dev_t *chip);

/**
 * @brief Configure the MCU-side GPIOs used by the INT1 / INT2 lines.
 */
void bma400_init_interrupt_gpios(bma400_dev_t *chip);

/**
 * @brief Set the device's power mode (sleep / low-power / normal).
 */
bma400_error_t bma400_set_power_mode(bma400_dev_t *chip, bma400_power_mode_t mode);

/**
 * @brief Apply an array of sensor configurations.
 *
 * Currently supports @ref BMA400_SENSOR_ACCEL and @ref BMA400_SENSOR_STEP_COUNTER.
 */
bma400_error_t bma400_set_sensor_conf(bma400_dev_t *chip,
                                      const bma400_sensor_cfg_t *cfgs,
                                      uint8_t n);

/**
 * @brief Read back the current sensor configurations. The caller must pre-fill
 *        @p cfgs[i].type so the driver knows which feature to read.
 */
bma400_error_t bma400_get_sensor_conf(bma400_dev_t *chip,
                                      bma400_sensor_cfg_t *cfgs,
                                      uint8_t n);

/**
 * @brief Enable / disable interrupt sources (INT_CONFIG0 and INT_CONFIG1).
 */
bma400_error_t bma400_enable_interrupt(bma400_dev_t *chip,
                                       const bma400_int_enable_t *ints,
                                       uint8_t n);

/**
 * @brief Read and combine INT_STAT0 / INT_STAT1 into a single 16-bit value.
 *        Low byte is INT_STAT0, high byte is INT_STAT1. Use @ref
 *        BMA400_ASSERTED_STEP_INT etc. to test individual flags.
 */
bma400_error_t bma400_get_interrupt_status(bma400_dev_t *chip, uint16_t *int_status);

/**
 * @brief Read the step counter and the current detected activity (still / walking / running).
 */
bma400_error_t bma400_get_steps_counted(bma400_dev_t *chip,
                                        uint32_t *step_count,
                                        bma400_activity_t *activity);

#endif /* BMA400_H_ */
