/**
 * @file   bma400.c
 * @brief  Portable part of the Bosch BMA400 accelerometer driver.
 *
 * This file implements the device-side logic: chip-id probing, soft reset,
 * power-mode changes, sensor/interrupt configuration, interrupt status
 * retrieval and step counter readout. It depends on three platform-specific
 * hooks (declared below) that live in separate port files so the driver can
 * be moved between MCUs without touching this translation unit.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "bma400.h"
#include "API_uart.h"

/**
 * Platform hooks, supplied by the interface port (bma400_i2c_port.c,
 * bma400_delay.c, ...). Kept as extern declarations on purpose: that way the
 * driver does not need to include any HAL header.
 */
extern void bma400_delay_us(uint32_t ms);
extern bma400_error_t bma400_read_data(const bma400_dev_t *chip, uint8_t reg_addr, size_t size, uint8_t *payload);
extern bma400_error_t bma400_send_data(const bma400_dev_t *chip, uint8_t reg_addr, size_t size, uint8_t *payload);

bma400_error_t bma400_chip_id(bma400_dev_t *chip, uint8_t *chip_id) {
    if(chip == NULL || chip_id == NULL) {
        return BMA400_INVAL;
    }
    ///< Fast-path: cached from a previous successful probe.
    if(chip->chip_id != 0) {
        *chip_id = chip->chip_id;
        return BMA400_OK;
    }
    uint8_t tmp_chip_id;
    bma400_error_t status = bma400_read_data(chip, BMA400_CHIP_ID_REG, sizeof(uint8_t), &tmp_chip_id);
    if(status != BMA400_OK) {
        return status;
    }
    ///< The returned value must match the datasheet constant; a different value
    ///< means we are talking to a different sensor on the bus.
    if(tmp_chip_id != BMA400_CHIP_ID) {
        return BMA400_READ_ERROR;
    }
    chip->chip_id = tmp_chip_id;
    *chip_id = tmp_chip_id;
    return BMA400_OK;
}

bma400_error_t bma400_send_cmd(bma400_dev_t *chip, uint8_t cmd) {
    if(chip == NULL) {
        return BMA400_INVAL;
    }
    ///< The CMD register only accepts a new command when STATUS.cmd_rdy == 1;
    ///< writing while the device is still busy would silently drop the command.
    uint8_t status;
    if(bma400_read_data(chip, BMA400_STATUS_REG, sizeof(uint8_t), &status) != BMA400_OK) {
        return BMA400_READ_ERROR;
    }

    if(!IS_BIT_SET(status, BMA400_CMD_RDY_STAT)) {
        return BMA400_BUSY;
    }

    return bma400_send_data(chip, BMA400_CMD_REG, sizeof(uint8_t), &cmd);
}


bma400_error_t bma400_soft_reset(bma400_dev_t *chip) {
    if(chip == NULL) {
        return BMA400_INVAL;
    }
    uint8_t cmd = BMA400_CMD_SOFT_RESET;
    bma400_error_t res = bma400_send_cmd(chip, cmd);
    ///< The datasheet requires a small settling time before the device accepts
    ///< further traffic after a soft reset.
    if(res == BMA400_OK) {
        bma400_delay_us(BMA400_SOFT_RESET_DELAY);
    }
    return res;
}


bma400_error_t bma400_clear_steps(bma400_dev_t *chip) {
    if(chip == NULL) {
        return BMA400_INVAL;
    }
    uint8_t cmd = BMA400_CMD_STEP_CNT_CLEAR;
    bma400_error_t ret = bma400_send_cmd(chip, cmd);
    return ret;
}


bma400_error_t bma400_configure_interrupts(bma400_dev_t *chip) {
    if(chip == NULL) {
        return BMA400_INVAL;
    }
    ///< INT12_IO_CTRL holds the electrical characteristics (level polarity and
    ///< driver type) of the two INT pins. Both halves of the register are
    ///< written together; unused channels leave their bits at 0.
    uint8_t int_config = 0;
    if(chip->int1_config != NULL) {
        int_config |= (chip->int1_config->active_high) ? BMA400_INT1_OUTPUT_LVL : 0x00;
        int_config |= (chip->int1_config->open_drain) ? BMA400_INT1_OUTPUT_OD : 0x00;
    }
    if(chip->int2_config != NULL) {
        int_config |= (chip->int2_config->active_high) ? BMA400_INT2_OUTPUT_LVL : 0x00;
        int_config |= (chip->int2_config->open_drain) ? BMA400_INT2_OUTPUT_OD : 0x00;
    }
    ///< Also configure the MCU-side GPIOs (EXTI, pull-ups, etc.) so the two
    ///< ends of the line are coherent.
    bma400_init_interrupt_gpios(chip);
    return bma400_send_data(chip, BMA400_INT12_IO_CTRL_REG, sizeof(uint8_t), &int_config);
}

bma400_error_t bma400_set_power_mode(bma400_dev_t *chip, bma400_power_mode_t mode) {
    if(chip == NULL) {
        return BMA400_INVAL;
    }
    ///< ACC_CONFIG0 also holds filter and OSR fields; read-modify-write so we
    ///< only touch POWER_MODE_CONF and preserve the rest.
    uint8_t reg;
    bma400_error_t ret = bma400_read_data(chip, BMA400_ACC_CONFIG0_REG, sizeof(uint8_t), &reg);
    if(ret != BMA400_OK) {
        return ret;
    }
    reg = (reg & ~BMA400_POWER_MODE_CONF) | FIELD_PREP(BMA400_POWER_MODE_CONF, (uint8_t)mode);
    return bma400_send_data(chip, BMA400_ACC_CONFIG0_REG, sizeof(uint8_t), &reg);
}

/**
 * @brief Route the step-detector interrupt to INT1, INT2, both or none by
 *        updating ACC_INT12_MAP. The step mapping is unique in that both
 *        channels share a single register (unlike the Gen/DRDY mappings).
 *
 * @param[in,out] chip BMA400 device descriptor
 * @param[in]     chan Channel selector (INT1 / INT2 / both / unmapped)
 * @return BMA400_OK on success, or the underlying error from the bus transfer
 */
static bma400_error_t map_step_int_pin(bma400_dev_t *chip, bma400_int_chan_t chan) {
    if(chip == NULL) {
        return BMA400_INVAL;
    }
    uint8_t reg;
    bma400_error_t ret = bma400_read_data(chip, BMA400_ACC_INT12_MAP_REG, sizeof(uint8_t), &reg);
    if(ret != BMA400_OK) {
        return ret;
    }
    reg &= ~(BMA400_STEP_INT1_MAP | BMA400_STEP_INT2_MAP);
    if(chan == BMA400_INT_CHANNEL_1 || chan == BMA400_MAP_BOTH_INT_PINS) {
        reg |= BMA400_STEP_INT1_MAP;
    }
    if(chan == BMA400_INT_CHANNEL_2 || chan == BMA400_MAP_BOTH_INT_PINS) {
        reg |= BMA400_STEP_INT2_MAP;
    }
    return bma400_send_data(chip, BMA400_ACC_INT12_MAP_REG, sizeof(uint8_t), &reg);
}

/**
 * @brief Route the data-ready (accelerometer) interrupt to INT1, INT2, both or
 *        none. DRDY has a dedicated bit in each of INT1_MAP and INT2_MAP, so
 *        the function reads both registers, clears the DRDY bit, and sets it
 *        according to the requested channel.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @param[in]     chan Channel selector (INT1 / INT2 / both / unmapped)
 * @return BMA400_OK on success, or the underlying error from any of the bus
 *         transfers involved.
 */
static bma400_error_t map_drdy_int_pin(bma400_dev_t *chip, bma400_int_chan_t chan) {
    if(chip == NULL) {
        return BMA400_INVAL;
    }
    uint8_t m1, m2;
    bma400_error_t ret = bma400_read_data(chip, BMA400_ACC_INT1_MAP_REG, sizeof(uint8_t), &m1);
    if(ret != BMA400_OK) {
        return ret;
    }
    ret = bma400_read_data(chip, BMA400_ACC_INT2_MAP_REG, sizeof(uint8_t), &m2);
    if(ret != BMA400_OK) {
        return ret;
    }
    m1 &= ~BMA400_DRDY_INT_MAP;
    m2 &= ~BMA400_DRDY_INT_MAP;
    if(chan == BMA400_INT_CHANNEL_1 || chan == BMA400_MAP_BOTH_INT_PINS) {
        m1 |= BMA400_DRDY_INT_MAP;
    }
    if(chan == BMA400_INT_CHANNEL_2 || chan == BMA400_MAP_BOTH_INT_PINS) {
        m2 |= BMA400_DRDY_INT_MAP;
    }
    ret = bma400_send_data(chip, BMA400_ACC_INT1_MAP_REG, sizeof(uint8_t), &m1);
    if(ret != BMA400_OK) {
        return ret;
    }
    return bma400_send_data(chip, BMA400_ACC_INT2_MAP_REG, sizeof(uint8_t), &m2);
}

/**
 * @brief Apply an accelerometer configuration to ACC_CONFIG0/1/2.
 *        ACC_CONFIG0 is touched read-modify-write because it also holds
 *        POWER_MODE_CONF (managed by @ref bma400_set_power_mode). After the
 *        three config registers are written, the DRDY interrupt is routed to
 *        the requested pin through @ref map_drdy_int_pin.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @param[in]     cfg  Caller-owned accelerometer configuration (range, ODR,
 *                     OSR, filter selection, int channel, ...)
 * @return BMA400_OK on success, or the underlying error from any of the bus
 *         transfers involved.
 */
static bma400_error_t set_accel_conf(bma400_dev_t *chip, const bma400_acc_conf_t *cfg) {
    if(chip == NULL || cfg == NULL) {
        return BMA400_INVAL;
    }
    uint8_t reg0;
    bma400_error_t ret = bma400_read_data(chip, BMA400_ACC_CONFIG0_REG, sizeof(uint8_t), &reg0);
    if(ret != BMA400_OK) {
        return ret;
    }
    /* Preserve POWER_MODE_CONF (set via bma400_set_power_mode). */
    reg0 &= ~(BMA400_FILT1_BW | BMA400_OSR_LP);
    reg0 |= FIELD_PREP(BMA400_FILT1_BW, cfg->filt1_bw);
    reg0 |= FIELD_PREP(BMA400_OSR_LP,   cfg->osr_lp);
    ret = bma400_send_data(chip, BMA400_ACC_CONFIG0_REG, sizeof(uint8_t), &reg0);
    if(ret != BMA400_OK) {
        return ret;
    }

    uint8_t reg1 = FIELD_PREP(BMA400_ACC_RNG, (uint8_t)cfg->range)
                 | FIELD_PREP(BMA400_ACC_OSR, cfg->osr)
                 | FIELD_PREP(BMA400_ACC_ODR, (uint8_t)cfg->odr);
    ret = bma400_send_data(chip, BMA400_ACC_CONFIG1_REG, sizeof(uint8_t), &reg1);
    if(ret != BMA400_OK) {
        return ret;
    }

    uint8_t reg2 = FIELD_PREP(BMA400_DATA_SRC, (uint8_t)cfg->data_src);
    ret = bma400_send_data(chip, BMA400_ACC_CONFIG2_REG, sizeof(uint8_t), &reg2);
    if(ret != BMA400_OK) {
        return ret;
    }

    return map_drdy_int_pin(chip, cfg->int_chan);
}

static bma400_error_t get_accel_conf(bma400_dev_t *chip, bma400_acc_conf_t *cfg) {
    if(chip == NULL || cfg == NULL) {
        return BMA400_INVAL;
    }
    uint8_t regs[3];
    bma400_error_t ret = bma400_read_data(chip, BMA400_ACC_CONFIG0_REG, sizeof(regs), regs);
    if(ret != BMA400_OK) {
        return ret;
    }
    cfg->filt1_bw = FIELD_GET(BMA400_FILT1_BW, regs[0]);
    cfg->osr_lp   = FIELD_GET(BMA400_OSR_LP,   regs[0]);
    cfg->range    = (bma400_acc_rng_t)   FIELD_GET(BMA400_ACC_RNG, regs[1]);
    cfg->osr      =                      FIELD_GET(BMA400_ACC_OSR, regs[1]);
    cfg->odr      = (bma400_acc_odr_t)   FIELD_GET(BMA400_ACC_ODR, regs[1]);
    cfg->data_src = (bma400_acc_filter_t)FIELD_GET(BMA400_DATA_SRC, regs[2]);

    uint8_t m1, m2;
    ret = bma400_read_data(chip, BMA400_ACC_INT1_MAP_REG, sizeof(uint8_t), &m1);
    if(ret != BMA400_OK) {
        return ret;
    }
    ret = bma400_read_data(chip, BMA400_ACC_INT2_MAP_REG, sizeof(uint8_t), &m2);
    if(ret != BMA400_OK) {
        return ret;
    }
    bool on1 = IS_BIT_SET(m1, BMA400_DRDY_INT_MAP);
    bool on2 = IS_BIT_SET(m2, BMA400_DRDY_INT_MAP);
    if(on1 && on2)      cfg->int_chan = BMA400_MAP_BOTH_INT_PINS;
    else if(on1)        cfg->int_chan = BMA400_INT_CHANNEL_1;
    else if(on2)        cfg->int_chan = BMA400_INT_CHANNEL_2;
    else                cfg->int_chan = BMA400_UNMAP_INT_PIN;
    return BMA400_OK;
}

/* Route generic interrupt 1 or 2 (selected by @p gen_map_bit) to INT1 / INT2. */
static bma400_error_t map_gen_int_pin(bma400_dev_t *chip, uint8_t gen_map_bit, bma400_int_chan_t chan) {
    if(chip == NULL) {
        return BMA400_INVAL;
    }
    uint8_t m1, m2;
    bma400_error_t ret = bma400_read_data(chip, BMA400_ACC_INT1_MAP_REG, sizeof(uint8_t), &m1);
    if(ret != BMA400_OK) {
        return ret;
    }
    ret = bma400_read_data(chip, BMA400_ACC_INT2_MAP_REG, sizeof(uint8_t), &m2);
    if(ret != BMA400_OK) {
        return ret;
    }
    m1 &= ~gen_map_bit;
    m2 &= ~gen_map_bit;
    if(chan == BMA400_INT_CHANNEL_1 || chan == BMA400_MAP_BOTH_INT_PINS) {
        m1 |= gen_map_bit;
    }
    if(chan == BMA400_INT_CHANNEL_2 || chan == BMA400_MAP_BOTH_INT_PINS) {
        m2 |= gen_map_bit;
    }
    ret = bma400_send_data(chip, BMA400_ACC_INT1_MAP_REG, sizeof(uint8_t), &m1);
    if(ret != BMA400_OK) {
        return ret;
    }
    return bma400_send_data(chip, BMA400_ACC_INT2_MAP_REG, sizeof(uint8_t), &m2);
}

/**
 * @brief Pack a Gen1/Gen2 interrupt configuration into the 11 consecutive
 *        registers that start at @p base_reg and ship it to the device.
 *        The reference-axis thresholds (int_thres_ref_x/y/z) are 12-bit values
 *        split across two registers each (low byte + low nibble of the next).
 *
 * @param[in,out] chip     BMA400 device descriptor; required to run the I2C transfer
 * @param[in]     base_reg First register of the Gen1/Gen2 config block
 * @param[in]     map_bit  Mask bit used to map the interrupt to INT1/INT2 pins
 * @param[in]     cfg      Caller-owned configuration to write
 * @return BMA400_OK on success, or the underlying error from the bus transfer
 *         or from map_gen_int_pin().
 */
static bma400_error_t set_gen_int_conf(bma400_dev_t *chip, uint8_t base_reg, uint8_t map_bit,
                                       const bma400_gen_int_conf_t *cfg) {
    if(chip == NULL || cfg == NULL) {
        return BMA400_INVAL;
    }
    uint8_t regs[11];
    regs[0] = (cfg->axes_sel & (BMA400_AXIS_X_EN | BMA400_AXIS_Y_EN | BMA400_AXIS_Z_EN))
            | FIELD_PREP(BMA400_GEN_DATA_SRC, cfg->data_src)
            | FIELD_PREP(BMA400_GEN_ACT_REF_UPDATE, (uint8_t)cfg->ref_update)
            | FIELD_PREP(BMA400_GEN_ACT_HYST, (uint8_t)cfg->hysteresis);
    regs[1] = FIELD_PREP(BMA400_GEN_CRITERION_SEL, cfg->criterion_sel)
            | FIELD_PREP(BMA400_GEN_COMB_SEL, cfg->evaluate_axes);
    regs[2] = cfg->gen_int_thres;
    /* gen_int_dur is a 16-bit count of filter samples, big-endian on the wire. */
    regs[3] = (uint8_t)((cfg->gen_int_dur >> 8) & 0xFF);
    regs[4] = (uint8_t)(cfg->gen_int_dur & 0xFF);
    /* 12-bit reference values: full low byte + low nibble in the high byte. */
    regs[5] = (uint8_t)(cfg->int_thres_ref_x & 0xFF);
    regs[6] = (uint8_t)((cfg->int_thres_ref_x >> 8) & 0x0F);
    regs[7] = (uint8_t)(cfg->int_thres_ref_y & 0xFF);
    regs[8] = (uint8_t)((cfg->int_thres_ref_y >> 8) & 0x0F);
    regs[9] = (uint8_t)(cfg->int_thres_ref_z & 0xFF);
    regs[10] = (uint8_t)((cfg->int_thres_ref_z >> 8) & 0x0F);
    bma400_error_t ret = bma400_send_data(chip, base_reg, sizeof(regs), regs);
    if(ret != BMA400_OK) {
        return ret;
    }
    return map_gen_int_pin(chip, map_bit, cfg->int_chan);
}

/**
 * @brief Read back a Gen1/Gen2 interrupt configuration from the 11 consecutive
 *        registers starting at @p base_reg, and resolve the current INT pin
 *        mapping by inspecting the corresponding bit in INT1_MAP / INT2_MAP.
 *
 * @param[in,out] chip     BMA400 device descriptor
 * @param[in]     base_reg First register of the Gen1/Gen2 config block
 * @param[in]     map_bit  Mask bit used to probe the interrupt mapping in
 *                         INT1_MAP / INT2_MAP
 * @param[out]    cfg      Caller-owned configuration struct filled on success
 * @return BMA400_OK on success, BMA400_INVAL if a required pointer is NULL,
 *         or the underlying error from any of the bus transfers involved.
 */
static bma400_error_t get_gen_int_conf(bma400_dev_t *chip, uint8_t base_reg, uint8_t map_bit,
                                       bma400_gen_int_conf_t *cfg) {
    if(chip == NULL || cfg == NULL) {
        return BMA400_INVAL;
    }
    uint8_t regs[BMA400_GENxINT_CONFIG_SIZE];
    bma400_error_t ret = bma400_read_data(chip, base_reg, ARRAY_SIZE(regs), regs);
    if(ret != BMA400_OK) {
        return ret;
    }
    cfg->axes_sel      = regs[0] & (BMA400_AXIS_X_EN | BMA400_AXIS_Y_EN | BMA400_AXIS_Z_EN);
    cfg->data_src      = FIELD_GET(BMA400_GEN_DATA_SRC, regs[0]);
    cfg->ref_update    = (bma400_gen_act_ref_update_t)FIELD_GET(BMA400_GEN_ACT_REF_UPDATE, regs[0]);
    cfg->hysteresis    = (bma400_act_hyst_conf_t)FIELD_GET(BMA400_GEN_ACT_HYST, regs[0]);
    cfg->criterion_sel = FIELD_GET(BMA400_GEN_CRITERION_SEL, regs[1]);
    cfg->evaluate_axes = FIELD_GET(BMA400_GEN_COMB_SEL, regs[1]);
    cfg->gen_int_thres = regs[2];
    cfg->gen_int_dur   = ((uint16_t)regs[3] << 8) | regs[4];
    cfg->int_thres_ref_x = ((uint16_t)(regs[6] & 0x0F) << 8) | regs[5];
    cfg->int_thres_ref_y = ((uint16_t)(regs[8] & 0x0F) << 8) | regs[7];
    cfg->int_thres_ref_z = ((uint16_t)(regs[10] & 0x0F) << 8) | regs[9];

    uint8_t m1, m2;
    ret = bma400_read_data(chip, BMA400_ACC_INT1_MAP_REG, sizeof(uint8_t), &m1);
    if(ret != BMA400_OK) {
        return ret;
    }
    ret = bma400_read_data(chip, BMA400_ACC_INT2_MAP_REG, sizeof(uint8_t), &m2);
    if(ret != BMA400_OK) {
        return ret;
    }
    bool on1 = IS_BIT_SET(m1, map_bit);
    bool on2 = IS_BIT_SET(m2, map_bit);
    if(on1 && on2)      cfg->int_chan = BMA400_MAP_BOTH_INT_PINS;
    else if(on1)        cfg->int_chan = BMA400_INT_CHANNEL_1;
    else if(on2)        cfg->int_chan = BMA400_INT_CHANNEL_2;
    else                cfg->int_chan = BMA400_UNMAP_INT_PIN;
    return BMA400_OK;
}

/**
 * @brief Read back the step-counter INT pin mapping from ACC_INT12_MAP.
 *        The step-counter has no other configuration bits exposed by this
 *        driver, so only @ref bma400_step_int_conf_t::int_chan is populated.
 *
 * @param[in,out] chip BMA400 device descriptor
 * @param[out]    cfg  Caller-owned configuration struct; only int_chan is
 *                     updated on success.
 * @return BMA400_OK on success, or the underlying error from the bus transfer.
 */
static bma400_error_t get_step_cnt_conf(bma400_dev_t *chip, bma400_step_int_conf_t *cfg) {
    if(chip == NULL || cfgs == NULL) {
        return BMA400_INVAL;
    }
    uint8_t reg;
    bma400_error_t ret = bma400_read_data(chip, BMA400_ACC_INT12_MAP_REG, sizeof(uint8_t), &reg);
    if(ret != BMA400_OK) {
        return ret;
    }
    bool on1 = IS_BIT_SET(reg, BMA400_STEP_INT1_MAP);
    bool on2 = IS_BIT_SET(reg, BMA400_STEP_INT2_MAP);
    if(on1 && on2)      cfg->int_chan = BMA400_MAP_BOTH_INT_PINS;
    else if(on1)        cfg->int_chan = BMA400_INT_CHANNEL_1;
    else if(on2)        cfg->int_chan = BMA400_INT_CHANNEL_2;
    else                cfg->int_chan = BMA400_UNMAP_INT_PIN;
    return BMA400_OK;
}

bma400_error_t bma400_set_sensor_conf(bma400_dev_t *chip, const bma400_sensor_cfg_t *cfgs, uint8_t n) {
    if(chip == NULL || cfgs == NULL || n == 0) {
        return BMA400_INVAL;
    }
    for(uint8_t i = 0; i < n; i++) {
        bma400_error_t ret;
        switch(cfgs[i].type) {
            case BMA400_SENSOR_ACCEL:
                ret = set_accel_conf(chip, &cfgs[i].param.accel);
                break;
            case BMA400_SENSOR_STEP_COUNTER:
                ret = map_step_int_pin(chip, cfgs[i].param.step_cnt.int_chan);
                break;
            case BMA400_SENSOR_GEN1:
                ret = set_gen_int_conf(chip, BMA400_GEN1INT_CONFIG_REG,
                                       BMA400_GEN1_INT_MAP, &cfgs[i].param.gen_int);
                break;
            default:
                /* Other sensor types (tap, act_ch, gen2, orient) are
                   not implemented yet. */
                return BMA400_INVAL;
        }
        if(ret != BMA400_OK) {
            return ret;
        }
    }
    return BMA400_OK;
}

bma400_error_t bma400_get_sensor_conf(bma400_dev_t *chip, bma400_sensor_cfg_t *cfgs, uint8_t n) {
    if(chip == NULL || cfgs == NULL || n == 0) {
        return BMA400_INVAL;
    }
    for(uint8_t i = 0; i < n; i++) {
        bma400_error_t ret;
        switch(cfgs[i].type) {
            case BMA400_SENSOR_ACCEL:
                ret = get_accel_conf(chip, &cfgs[i].param.accel);
                break;
            case BMA400_SENSOR_STEP_COUNTER:
                ret = get_step_cnt_conf(chip, &cfgs[i].param.step_cnt);
                break;
            case BMA400_SENSOR_GEN1:
                ret = get_gen_int_conf(chip, BMA400_GEN1INT_CONFIG_REG,
                                       BMA400_GEN1_INT_MAP, &cfgs[i].param.gen_int);
                break;
            default:
                return BMA400_INVAL;
        }
        if(ret != BMA400_OK) {
            return ret;
        }
    }
    return BMA400_OK;
}

bma400_error_t bma400_enable_interrupt(bma400_dev_t *chip, const bma400_int_enable_t *ints, uint8_t n) {
    if(chip == NULL || ints == NULL || n == 0) {
        return BMA400_INVAL;
    }
    uint8_t cfg[2]; /* INT_CONFIG0 (0x1F), INT_CONFIG1 (0x20) */
    bma400_error_t ret = bma400_read_data(chip, BMA400_ACC_INT_CONFIG0_REG, sizeof(cfg), cfg);
    if(ret != BMA400_OK) {
        return ret;
    }

    for(uint8_t i = 0; i < n; i++) {
        bool on = (ints[i].conf != 0);
        uint8_t bit = 0;
        uint8_t *reg = NULL;
        switch(ints[i].type) {
            case BMA400_DATA_READY_INT_EN:      reg = &cfg[0]; bit = BMA400_DRDY_INT_EN;       break;
            case BMA400_FIFO_WM_INT_EN:         reg = &cfg[0]; bit = BMA400_FWM_INT_EN;        break;
            case BMA400_FIFO_FULL_INT_EN:       reg = &cfg[0]; bit = BMA400_FFULL_INT_EN;      break;
            case BMA400_GEN2_INT_EN:            reg = &cfg[0]; bit = BMA400_GEN2_INT_EN_FLAG; break;
            case BMA400_GEN1_INT_EN:            reg = &cfg[0]; bit = BMA400_GEN1_INT_EN_FLAG; break;
            case BMA400_ORIENT_CHANGE_INT_EN:   reg = &cfg[0]; bit = BMA400_ORIENTCH_INT_EN;   break;
            case BMA400_LATCH_INT_EN:           reg = &cfg[1]; bit = BMA400_LATCH_INT;         break;
            case BMA400_ACTIVITY_CHANGE_INT_EN: reg = &cfg[1]; bit = BMA400_ACTCH_INT_EN;      break;
            case BMA400_DOUBLE_TAP_INT_EN:      reg = &cfg[1]; bit = BMA400_D_TAP_INT_EN;      break;
            case BMA400_SINGLE_TAP_INT_EN:      reg = &cfg[1]; bit = BMA400_S_TAP_INT_EN;      break;
            case BMA400_STEP_COUNTER_INT_EN:    reg = &cfg[1]; bit = BMA400_STEP_INT_EN;       break;
            default:
                /* AUTO_WAKEUP lives in AUTOWAKEUP_1; not handled here yet. */
                return BMA400_INVAL;
        }
        if(on)  *reg |=  bit;
        else    *reg &= ~bit;
    }
    return bma400_send_data(chip, BMA400_ACC_INT_CONFIG0_REG, sizeof(cfg), cfg);
}

bma400_error_t bma400_get_interrupt_status(bma400_dev_t *chip, uint16_t *int_status) {
    if(chip == NULL || int_status == NULL) {
        return BMA400_INVAL;
    }
    uint8_t regs[3]; /* INT_STAT0 (0x0E), INT_STAT1 (0x0F), INT_STAT2 (0x10) */
    bma400_error_t ret = bma400_read_data(chip, BMA400_INT_STAT0_REG, sizeof(regs), regs);
    if(ret != BMA400_OK) {
        return ret;
    }
    ///< Low byte = INT_STAT0, high byte = INT_STAT1 (matches Bosch API shape).
    ///< INT_STAT2 (ACTCH flags) is not combined here yet.
    *int_status = ((uint16_t)regs[1] << 8) | regs[0];
    return BMA400_OK;
}

bma400_error_t bma400_get_steps_counted(bma400_dev_t *chip, uint32_t *step_count, bma400_activity_t *activity) {
    if(chip == NULL || step_count == NULL || activity == NULL) {
        return BMA400_INVAL;
    }
    uint8_t regs[4]; /* STEP_CNT0..2 + STEP_STAT */
    bma400_error_t ret = bma400_read_data(chip, BMA400_STEP_CNT0_REG, sizeof(regs), regs);
    if(ret != BMA400_OK) {
        return ret;
    }
    *step_count = (uint32_t)regs[0]
                | ((uint32_t)regs[1] << 8)
                | ((uint32_t)regs[2] << 16);
    *activity = (bma400_activity_t)FIELD_GET(BMA400_STEP_ACTIVITY, regs[3]);
    return BMA400_OK;
}

bma400_error_t bma400_init(bma400_dev_t *chip) {
    if(chip == NULL) {
        return BMA400_INVAL;
    }

    // read chip id to verify that communication is working and cache it in the chip struct
    if(bma400_chip_id(chip, &chip->chip_id) != BMA400_OK) {
        return BMA400_READ_ERROR;
    }

    bma400_error_t status = bma400_soft_reset(chip);
    if(status != BMA400_OK) {
        return status;
    }

    status = bma400_configure_interrupts(chip);
    if(status != BMA400_OK) {
        return status;
    }

    return BMA400_OK;
}
