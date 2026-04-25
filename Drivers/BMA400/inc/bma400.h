/**
 * @file   bma400.h
 * @brief  Public API of the Bosch BMA400 3-axis accelerometer driver.
 *
 * The driver is split between this portable layer (device logic, register
 * packing and sensor configuration) and platform-specific port files that
 * provide the I2C transport, microsecond delay and interrupt-GPIO setup.
 * All functions return @ref bma400_error_t; BMA400_OK (== 0) means success.
 */
#ifndef BMA400_H_
#define BMA400_H_

#include <stddef.h>
#include <stdint.h>

#include "bma400_def.h"


/**
 * @brief Read the BMA400 chip-id register and cache it in @p chip on success.
 *        Subsequent calls return the cached value without hitting the bus.
 *
 * @param[in,out] chip    BMA400 device descriptor (chip_id field is updated)
 * @param[out]    chip_id Caller-owned byte; set to the read chip-id on success
 * @return BMA400_OK on success, BMA400_INVAL on NULL pointer, BMA400_READ_ERROR
 *         if the transfer fails or the returned value does not match
 *         @ref BMA400_CHIP_ID.
 */
bma400_error_t bma400_chip_id(bma400_dev_t *chip, uint8_t *chip_id);

/**
 * @brief Issue a command through the CMD register, refusing to write while
 *        the previous command is still pending (STATUS.cmd_rdy == 0).
 *
 * @param[in,out] chip BMA400 device descriptor
 * @param[in]     cmd  Command byte (see BMA400_CMD_* in bma400_def.h)
 * @return BMA400_OK on success, BMA400_INVAL on NULL @p chip, BMA400_BUSY
 *         if the device is still processing a previous command, or the
 *         underlying bus error.
 */
bma400_error_t bma400_send_cmd(bma400_dev_t *chip, uint8_t cmd);

/**
 * @brief Soft-reset the device (CMD 0xB6) and wait for the settling time
 *        mandated by the datasheet before returning.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @return BMA400_OK on success, BMA400_INVAL on NULL @p chip, or the error
 *         propagated from @ref bma400_send_cmd.
 */
bma400_error_t bma400_soft_reset(bma400_dev_t *chip);

/**
 * @brief Clear the step counter (CMD 0xB1). The next step-counter interrupt
 *        will report a count of zero plus any new steps accumulated since.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @return BMA400_OK on success, BMA400_INVAL on NULL @p chip, or the error
 *         propagated from @ref bma400_send_cmd.
 */
bma400_error_t bma400_clear_steps(bma400_dev_t *chip);

/**
 * @brief Initialise the device: verify the chip-id, perform a soft reset and
 *        apply the INT pin electrical configuration stored in
 *        @p chip->int1_config / int2_config. Sensor-level configuration
 *        (accel, step, gen1, ...) is applied afterwards via
 *        @ref bma400_set_sensor_conf.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @return BMA400_OK on success, BMA400_INVAL on NULL @p chip, or the first
 *         error encountered while bringing up the device.
 */
bma400_error_t bma400_init(bma400_dev_t *chip);

/**
 * @brief Apply the INT1 / INT2 electrical configuration (level polarity and
 *        driver type) stored in @p chip->int1_config and @p chip->int2_config.
 *        Also invokes @ref bma400_init_interrupt_gpios so the MCU-side pins
 *        are coherent with the sensor-side settings.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @return BMA400_OK on success, BMA400_INVAL on NULL @p chip, or the error
 *         propagated from the underlying bus transfer.
 */
bma400_error_t bma400_configure_interrupts(bma400_dev_t *chip);

/**
 * @brief Configure the MCU-side GPIOs used by the INT1 / INT2 lines, based on
 *        the descriptors in @p chip->int1_config and @p chip->int2_config.
 *        A NULL descriptor means the corresponding line is unused.
 *
 * @param[in] chip BMA400 device descriptor
 */
void bma400_init_interrupt_gpios(bma400_dev_t *chip);

/**
 * @brief Set the device's power mode (sleep / low-power / normal). This is a
 *        read-modify-write on ACC_CONFIG0 so the filter/OSR fields are
 *        preserved.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @param[in]     mode Requested power mode
 * @return BMA400_OK on success, BMA400_INVAL on NULL @p chip, or the error
 *         propagated from the underlying bus transfers.
 */
bma400_error_t bma400_set_power_mode(bma400_dev_t *chip, bma400_power_mode_t mode);

/**
 * @brief Apply an array of sensor configurations. Each element of @p cfgs is
 *        dispatched based on its @c type field. Currently supports
 *        @ref BMA400_SENSOR_ACCEL, @ref BMA400_SENSOR_STEP_COUNTER and
 *        @ref BMA400_SENSOR_GEN1.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @param[in]     cfgs Array of @p n caller-owned configurations
 * @param[in]     n    Number of valid entries in @p cfgs (>= 1)
 * @return BMA400_OK on success, BMA400_INVAL on NULL pointers, zero @p n or
 *         an unsupported sensor type, or the error propagated from the
 *         underlying bus transfers.
 */
bma400_error_t bma400_set_sensor_conf(bma400_dev_t *chip,
                                      const bma400_sensor_cfg_t *cfgs,
                                      uint8_t n);

/**
 * @brief Read back the current sensor configurations. The caller must
 *        pre-fill @p cfgs[i].type so the driver knows which feature block to
 *        read; the @c param union of each element is then populated.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @param[in,out] cfgs Array of @p n configurations. On entry each @c type
 *                     must be set; on success the corresponding @c param is
 *                     filled in.
 * @param[in]     n    Number of valid entries in @p cfgs (>= 1)
 * @return BMA400_OK on success, BMA400_INVAL on NULL pointers, zero @p n or
 *         an unsupported sensor type, or the error propagated from the
 *         underlying bus transfers.
 */
bma400_error_t bma400_get_sensor_conf(bma400_dev_t *chip,
                                      bma400_sensor_cfg_t *cfgs,
                                      uint8_t n);

/**
 * @brief Enable or disable individual interrupt sources in INT_CONFIG0 and
 *        INT_CONFIG1. The two registers are read, updated according to the
 *        @p ints array, and written back atomically (one block write).
 *
 * @param[in,out] chip BMA400 device descriptor
 * @param[in]     ints Array of @p n enable/disable requests
 * @param[in]     n    Number of valid entries in @p ints (>= 1)
 * @return BMA400_OK on success, BMA400_INVAL on NULL pointers, zero @p n or
 *         an unsupported interrupt type, or the error propagated from the
 *         underlying bus transfers.
 */
bma400_error_t bma400_enable_interrupt(bma400_dev_t *chip,
                                       const bma400_int_enable_t *ints,
                                       uint8_t n);

/**
 * @brief Read and combine INT_STAT0 / INT_STAT1 into a single 16-bit value.
 *        Low byte holds INT_STAT0, high byte holds INT_STAT1. Use the
 *        BMA400_*_INT_STAT / BMA400_STEP_INT_STAT masks to test individual
 *        flags. Reading the registers also clears any latched flags.
 *
 * @param[in,out] chip       BMA400 device descriptor
 * @param[out]    int_status Caller-owned 16-bit word; filled on success.
 * @return BMA400_OK on success, BMA400_INVAL on NULL pointer, or the error
 *         propagated from the underlying bus transfer.
 */
bma400_error_t bma400_get_interrupt_status(bma400_dev_t *chip, uint16_t *int_status);

/**
 * @brief Read the 24-bit step counter together with the current detected
 *        activity (still / walking / running) as reported by the BMA400's
 *        on-chip classifier in the STEP_STAT register.
 *
 * @param[in,out] chip       BMA400 device descriptor
 * @param[out]    step_count Caller-owned 32-bit word; the low 24 bits hold
 *                           the step count, the top byte is always 0.
 * @param[out]    activity   Caller-owned enum; filled with the latest
 *                           classification result on success.
 * @return BMA400_OK on success, BMA400_INVAL on NULL pointer, or the error
 *         propagated from the underlying bus transfer.
 */
bma400_error_t bma400_get_steps_counted(bma400_dev_t *chip,
                                        uint32_t *step_count,
                                        bma400_activity_t *activity);

#endif /* BMA400_H_ */
