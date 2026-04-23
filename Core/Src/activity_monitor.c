/**
 * @file    activity_monitor.c
 * @brief   FSM that orchestrates the BMA400 IMU, the LCD1602 display, the UART
 *          console and the user button (B1) to classify the user's activity
 *          (still / walking / running), count steps, detect free-fall events
 *          and escalate them to an EMERGENCY state if they are not cancelled
 *          in time.
 *
 * The module is driven from the main super-loop via activity_monitor_update().
 * No interrupts are required: the BMA400 status is polled from the loop and
 * its internal interrupt flags are kept latched until read.
 */

#include "API_i2c.h"
#include "API_uart.h"
#include "API_debounce.h"
#include "API_delay.h"
#include "activity_monitor.h"
#include "bma400.h"
#include "lcd1602.h"
#include "stm32f4xx_hal.h"

#define MIN_EMERGENCY_PRESSED_CNT 5      ///< button presses required to leave EMERGENCY
#define FREE_FALL_TIMEOUT 30000U         ///< free fall -> emergency timeout in ms (30 s)
#define FREE_FALL_ACCELERATION_THRESHOLD_MG 63U ///< free fall detection threshold in units of 8mg (|X|,|Y|,|Z| < 504 mg)
#define FREE_FALL_DETECTION_DURATION_MS 60U ///< free fall detection duration in units of 10ms (>= 600 ms)

#define BMA400_I2C_ADDRESS (0x15 << 1)   ///< BMA400 with SDO tied to GND
#define LCD1602_I2C_ADDRESS (0x27 << 1)  ///< PCF8574 backpack default

extern I2C_HandleTypeDef hi2c1;

static activity_monitor_error_t last_error = AM_OK;       ///< Last error (if any) of the FSM
static activity_monitor_state_t state = STILL;            ///< current state of the FSM
static bma400_activity_t imu_activity = BMA400_STILL;     ///< last activity reported by the IMU
static bool initialized = false, must_update_display = false;
static uint32_t steps = 0;                                ///< cached step count shown on the LCD
static uint16_t imu_int_status;                           ///< combined INT_STAT0/INT_STAT1 of the BMA400
static uint8_t emergency_button_pressed_counter = 0;      ///< presses accumulated while in EMERGENCY
static delay_t free_fall_emergency_timer;                 ///< non-blocking 30 s timer for FREE_FALL -> EMERGENCY

static i2c_port_t display_port = {
    .i2c_handler = &hi2c1,
    .address = LCD1602_I2C_ADDRESS,
};

static i2c_port_t imu_port = {
    .i2c_handler = &hi2c1,
    .address = BMA400_I2C_ADDRESS,
};

static bma400_dev_t imu = {
    .intf_ptr = &imu_port,
};

/**
 * Sensor-side configuration applied at init.
 * Three blocks are used:
 *   - STEP_COUNTER: routes the step interrupt and enables activity classification
 *   - ACCEL:        output data rate, range, filter
 *   - GEN1:         generic interrupt 1, used here as the free-fall detector
 */
static bma400_sensor_cfg_t imu_settings[3] = {
    { .type = BMA400_SENSOR_STEP_COUNTER },
    { .type = BMA400_SENSOR_ACCEL },
    { .type = BMA400_SENSOR_GEN1 },
};

/**
 * Interrupt sources enabled in INT_CONFIG0/INT_CONFIG1.
 * LATCH_INT keeps every flag set until INT_STAT is read, so the super-loop
 * can poll at its own pace without losing events.
 */
static bma400_int_enable_t imu_int_en[3] = {
    { .type = BMA400_STEP_COUNTER_INT_EN, .conf = 1 },
    { .type = BMA400_LATCH_INT_EN,        .conf = 1 },
    { .type = BMA400_GEN1_INT_EN,         .conf = 1 },
};

/**
 * LCD1602 (16x2) behind a PCF8574 I2C backpack.
 * row_offsets follow the HD44780 DDRAM layout: row 0 starts at 0x00, row 1 at 0x40.
 */
static lcd1602_data_t display = {
    .port = &display_port,
    .backlight_on = true,
    .disp_func = LCD1602_FUNCTION_SET_2LINE,
    .disp_ctrl = LCD1602_DISPLAY_ON,
    .disp_mode = LCD1602_ENTRY_MODE_INCREMENT,
    .num_lines = 2,
    .row_offsets = {0x00, 0x40, 0x00, 0x00},
};

/**
 * @brief Human-readable name of the current FSM state. Used for both the LCD
 *        (row 0) and the UART log.
 *
 * @return Pointer to a statically allocated string literal describing the
 *         current value of the module-level @ref state. Never NULL.
 */
static const char * state_to_str() {
    switch(state) {
        case STILL:
            return "Still";
        case WALKING:
            return "Walking";
        case RUNNING:
            return "Running";
        case FREE_FALL:
            return "Free fall";
        case EMERGENCY:
            return "Emergency";
        case ACTIVITY_ERROR:
            return "Error";
        default:
            return "Unknown";
    }
}

/**
 * @brief Refresh the LCD (state + step count) and mirror the same information
 *        through UART.
 *
 * Any failure reported by the LCD or UART drivers aborts the refresh and moves
 * the FSM to ACTIVITY_ERROR, recording the cause in @ref last_error.
 */
static void update_display(void)
{
    lcd1602_error_t display_ret = lcd1602_clear(&display);
    if(display_ret != LCD1602_OK) {
        uart_send_string("Failed to clear display\r\n");
        state = ACTIVITY_ERROR;
        last_error = AM_DISPLAY_WRITE_ERROR;
        return;
    }
    display_ret = lcd1602_set_cursor(&display, 0, 0);
    if(display_ret != LCD1602_OK) {
        uart_send_string("Failed to set cursor on display\r\n");
        state = ACTIVITY_ERROR;
        last_error = AM_DISPLAY_WRITE_ERROR;
        return;
    }
    display_ret = lcd1602_write_string(&display, state_to_str());
    if(display_ret != LCD1602_OK) {
        uart_send_string("Failed to write to display\r\n");
        state = ACTIVITY_ERROR;
        last_error = AM_DISPLAY_WRITE_ERROR;
        return;
    }
    display_ret = lcd1602_set_cursor(&display, 0, 1);
    if(display_ret != LCD1602_OK) {
        uart_send_string("Failed to set cursor on display\r\n");
        state = ACTIVITY_ERROR;
        last_error = AM_DISPLAY_WRITE_ERROR;
        return;
    }
    display_ret = lcd1602_write_fmt_string(&display, "Steps: %ld", steps);
    if(display_ret != LCD1602_OK) {
        uart_send_string("Failed to write to display\r\n");
        state = ACTIVITY_ERROR;
        last_error = AM_DISPLAY_WRITE_ERROR;
        return;
    }
    uart_error_t uart_ret = uart_send_formatted_string("Status %s \t steps: %ld\r\n", state_to_str(), steps);
    if(uart_ret != UART_OK) {
        state = ACTIVITY_ERROR;
        last_error = AM_UART_WRITE_ERROR;
    }
}

activity_monitor_error_t activity_monitor_init(void)
{
    if(uart_init() != UART_OK) {
        return AM_UART_INIT_ERROR;
    }
    lcd1602_error_t display_ret = lcd1602_init(&display);
    if(display_ret != LCD1602_OK) {
        uart_send_formatted_string("LCD1602 initialization failed (err %d)\r\n", (uint8_t)display_ret);
        return AM_DISPLAY_INIT_ERROR;
    }

    /// Verify chip-id, soft-reset and apply the defaults for the INT pins.
    bma400_error_t imu_ret = bma400_init(&imu);
    if(imu_ret != BMA400_OK) {
        uart_send_formatted_string("IMU initialization failed (err %d)\r\n", (uint8_t)imu_ret);
        return AM_IMU_INIT_ERROR;
    }
    ///< Pull the current sensor configuration to use it as a base and only
    ///< override the fields we care about.
    imu_ret = bma400_get_sensor_conf(&imu, imu_settings, ARRAY_SIZE(imu_settings));
    if(imu_ret != BMA400_OK) {
        uart_send_formatted_string("IMU get configuration failed (err %d)\r\n", (uint8_t)imu_ret);
        return AM_IMU_CONFIG_ERROR;
    }
    ///< Route the step-detector interrupt to INT1 (we poll the status register,
    ///< the physical pin is not wired to the MCU).
    imu_settings[0].param.step_cnt.int_chan = BMA400_INT_CHANNEL_1;
    /// Accelerometer running from acc_filt1 at 100 Hz, +/-4g.
    imu_settings[1].param.accel.odr = BMA400_ODR_100HZ;
    imu_settings[1].param.accel.range = BMA400_RNG_4G;
    imu_settings[1].param.accel.data_src = BMA400_ACC_FILT1;
    ///< Freefall detection via Gen1 interrupt (see https://community.bosch-sensortec.com/knowledge-base-pg631enp/post/bma400-accelerometer-design-guide-3yEbSHfrXTRME5a).
    ///< Triggers when |X|,|Y|,|Z| are all within +/-504 mg for >= 600 ms.
    ///< Duration is expressed in acc_filt2 samples (fixed 100 Hz, 10 ms each):
    ///<   60 samples * 10 ms = 600 ms   =>  H = 0.5 * 504mg * t^2 ~= 0.882 m.
    ///< This filters out walking/running bounces while still catching a real
    ///< fall from ~1.3 m.
    imu_settings[2].param.gen_int.axes_sel      = BMA400_AXIS_X_EN | BMA400_AXIS_Y_EN | BMA400_AXIS_Z_EN;
    imu_settings[2].param.gen_int.data_src      = BMA400_ACC_FILT2; // acc_filt2 (fixed 100 Hz)
    imu_settings[2].param.gen_int.ref_update    = BMA400_GEN_ACT_REFU_MANUAL;
    imu_settings[2].param.gen_int.hysteresis    = BMA400_GEN_ACT_HYST_NOT_ACTIVE;
    imu_settings[2].param.gen_int.criterion_sel = 0; // below threshold (inactivity)
    imu_settings[2].param.gen_int.evaluate_axes = 1; // AND: all axes must enter the zone
    imu_settings[2].param.gen_int.gen_int_thres = FREE_FALL_ACCELERATION_THRESHOLD_MG;
    imu_settings[2].param.gen_int.gen_int_dur   = FREE_FALL_DETECTION_DURATION_MS;
    imu_settings[2].param.gen_int.int_thres_ref_x = 0;
    imu_settings[2].param.gen_int.int_thres_ref_y = 0;
    imu_settings[2].param.gen_int.int_thres_ref_z = 0;
    imu_settings[2].param.gen_int.int_chan      = BMA400_INT_CHANNEL_1;
    imu_ret = bma400_set_sensor_conf(&imu, imu_settings, ARRAY_SIZE(imu_settings));
    if(imu_ret != BMA400_OK) {
        uart_send_formatted_string("IMU set configuration failed (err %d)\r\n", (uint8_t)imu_ret);
        return AM_IMU_CONFIG_ERROR;
    }
    imu_ret = bma400_set_power_mode(&imu, BMA400_NORMAL);
    if(imu_ret != BMA400_OK) {
        uart_send_formatted_string("IMU set power mode failed (err %d)\r\n", (uint8_t)imu_ret);
        return AM_IMU_CONFIG_ERROR;
    }
    imu_ret = bma400_enable_interrupt(&imu, imu_int_en, ARRAY_SIZE(imu_int_en));
    if(imu_ret != BMA400_OK) {
        uart_send_formatted_string("IMU enable interrupt failed (err %d)\r\n", (uint8_t)imu_ret);
        return AM_IMU_CONFIG_ERROR;
    }
    initialized = true;
    delay_init(&free_fall_emergency_timer, FREE_FALL_TIMEOUT);
    uart_send_string("activity monitor initialized. Walk/Run to change its state.\r\n");
    update_display();

    return AM_OK;
}

void activity_monitor_update(void)
{
    if(initialized == false) { return ;}
    ///< read_key() returns true exactly once per validated press and
    ///< clears the internal flag, so it is safe to consume it here.
    bool button_pressed = read_key();
    switch(state) {
        case STILL:
            ///< STILL is the only state where the button clears the step
            ///< counter (in WALKING/RUNNING it would discard valid data, and
            ///< in FREE_FALL/EMERGENCY the button means "cancel").
            if(button_pressed) {
                bma400_clear_steps(&imu);
                steps = 0;
                state = STILL;
                update_display();
                break;
            }
            /// fall through: STILL/WALKING/RUNNING share the same IMU polling */
        case WALKING:
        case RUNNING:
            if(bma400_get_interrupt_status(&imu, &imu_int_status) != BMA400_OK) {
                state = ACTIVITY_ERROR;
                last_error = AM_IMU_READ_ERROR;
                break;
            }
            ///< Gen1 fires on free-fall (|X|,|Y|,|Z| < 504 mg for 200 ms); it
            ///< takes precedence over the step-counter update to avoid masking
            ///< the alarm with an in-flight step.
            if(IS_BIT_SET(BMA400_GEN1_INT_STAT, imu_int_status) == true) {
                state = FREE_FALL;
                update_display();
                break;
            }
            if(FIELD_GET(BMA400_STEP_INT_STAT, imu_int_status) != 0x00) {
                bma400_activity_t tmp_activity;
                uint32_t tmp_steps;
                bma400_error_t imu_ret = bma400_get_steps_counted(&imu, &tmp_steps, &tmp_activity);
                if(imu_ret != BMA400_OK) {
                    state = ACTIVITY_ERROR;
                    last_error = AM_IMU_READ_ERROR;
                    break;
                }
                if(tmp_steps != steps) {
                    steps = tmp_steps;
                    must_update_display = true;
                }
                uart_send_formatted_string("activity detected imu int status 0x%02X\r\n", imu_int_status);
                ///< Map the IMU-reported activity to the FSM state.
                if(tmp_activity != imu_activity) {
                    must_update_display = true;
                    imu_activity = tmp_activity;
                    switch(imu_activity) {
                        case BMA400_STILL:
                            state = STILL;
                            break;
                        case BMA400_WALKING:
                            state = WALKING;
                            break;
                        case BMA400_RUNNING:
                            state = RUNNING;
                            break;
                        default:
                            state = ACTIVITY_ERROR;
                            break;
                    }
                }
                update_display();
            }
            break;
        case FREE_FALL:
            /// A single press cancels the alert and returns to STILL.
            if(button_pressed) {
                delay_stop(&free_fall_emergency_timer);
                state = STILL;
                update_display();
                uint16_t tmp_int_status;
                ///< Read INT_STAT so the latched Gen1 flag is cleared before
                ///< the next tick; otherwise we would re-enter FREE_FALL
                ///< immediately.
                if(bma400_get_interrupt_status(&imu, &tmp_int_status) != BMA400_OK) {
                    state = ACTIVITY_ERROR;
                    last_error = AM_IMU_READ_ERROR;
                    break;
                }
                break;
            }
            /// No press within FREE_FALL_TIMEOUT (30 s) -> escalate.
            if(true == delay_read(&free_fall_emergency_timer)) {
                state = EMERGENCY;
                update_display();
                break;
            }
            break;
        case EMERGENCY:
            ///< EMERGENCY is intentionally hard to exit: at least
            ///< MIN_EMERGENCY_PRESSED_CNT validated presses are required to
            ///< avoid accidental cancellations.
            if(button_pressed) {
                emergency_button_pressed_counter++;
            }
            if(emergency_button_pressed_counter >= MIN_EMERGENCY_PRESSED_CNT) {
                state = STILL;
                uint16_t tmp_int_status;
                ///< Read INT_STAT so the latched Gen1 flag is cleared before
                ///< the next tick; otherwise we would re-enter FREE_FALL
                ///< immediately.
                if(bma400_get_interrupt_status(&imu, &tmp_int_status) != BMA400_OK) {
                    state = ACTIVITY_ERROR;
                    last_error = AM_IMU_READ_ERROR;
                    break;
                }
                update_display();
            }
            break;
        case ACTIVITY_ERROR:
            /* Sticky state: only a MCU reset recovers from here. */
            break;
        default:
            return;
    }
}