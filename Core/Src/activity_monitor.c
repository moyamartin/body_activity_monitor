#include "API_i2c.h"
#include "API_uart.h"
#include "activity_monitor.h"
#include "bma400.h"
#include "lcd1602.h"
#include "stm32f4xx_hal.h"

#define BMA400_I2C_ADDRESS (0x15 << 1)
#define LCD1602_I2C_ADDRESS (0x27 << 1)

extern I2C_HandleTypeDef hi2c1;

static activity_monitor_error_t last_error = AM_OK;
static activity_monitor_state_t state = STILL;
static bma400_activity_t imu_activity = BMA400_STILL;
static bool initialized = false, must_update_display = false;
static uint32_t steps = 0;
static uint16_t imu_int_status;

static i2c_port_t display_port = {
    .i2c_handler = &hi2c1,
    .address = LCD1602_I2C_ADDRESS,
};

static i2c_port_t imu_port = {
    .i2c_handler = &hi2c1,
    .address = BMA400_I2C_ADDRESS, // BMA400 I2C address shifted left for HAL functions
};

static bma400_dev_t imu = {
    .intf_ptr = &imu_port,
};

static bma400_sensor_cfg_t imu_settings[2] = {
    { .type = BMA400_SENSOR_STEP_COUNTER },
    { .type = BMA400_SENSOR_ACCEL },
};

static bma400_int_enable_t imu_int_en[2] = {
    { .type = BMA400_STEP_COUNTER_INT_EN, .conf = 1 },
    { .type = BMA400_LATCH_INT_EN,        .conf = 1 },
};

static lcd1602_data_t display = {
    .port = &display_port,
    .backlight_on = true,
    .disp_func = LCD1602_FUNCTION_SET_2LINE,
    .disp_ctrl = LCD1602_DISPLAY_ON,
    .disp_mode = LCD1602_ENTRY_MODE_INCREMENT,
    .num_lines = 2,
    .row_offsets = {0x00, 0x40, 0x00, 0x00},
};

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

static void update_display(void)
{
    lcd1602_error_t display_ret = lcd1602_set_cursor(&display, 0, 0);
    if(display_ret != LCD1602_OK) {
        uart_send_string("Failed to set cursor on display\r\n");
        state = ACTIVITY_ERROR;
        last_error = AM_DISPLAY_WRITE_ERROR;
    }
    display_ret = lcd1602_write_string(&display, "Hello world!");
    if(display_ret != LCD1602_OK) {
        uart_send_string("Failed to write to display\r\n");
        state = ACTIVITY_ERROR;
        last_error = AM_DISPLAY_WRITE_ERROR;
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

    bma400_error_t imu_ret = bma400_init(&imu);
    if(imu_ret != BMA400_OK) {
        uart_send_formatted_string("IMU initialization failed (err %d)\r\n", (uint8_t)imu_ret);
        return AM_IMU_INIT_ERROR;
    }
    imu_ret = bma400_get_sensor_conf(&imu, imu_settings, ARRAY_SIZE(imu_settings));
    if(imu_ret != BMA400_OK) {
        uart_send_formatted_string("IMU get configuration failed (err %d)\r\n", (uint8_t)imu_ret);
        return AM_IMU_CONFIG_ERROR;
    }
    imu_settings[0].param.step_cnt.int_chan = BMA400_INT_CHANNEL_1;
    imu_settings[1].param.accel.odr = BMA400_ODR_100HZ;
    imu_settings[1].param.accel.range = BMA400_RNG_2G;
    imu_settings[1].param.accel.data_src = BMA400_ACC_FILT1;
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
    uart_send_string("activity monitor initialized. Walk/Run to change its state.\r\n");
    update_display();
    return AM_OK;
}

void activity_monitor_update(void)
{
    switch(state) {
        case STILL:
            /// here's the only state where the step counter can be reset to 0
            /// if the button is pressed
        case WALKING:
        case RUNNING:
            if(bma400_get_interrupt_status(&imu, &imu_int_status) != BMA400_OK) {
                state = ACTIVITY_ERROR;
                last_error = AM_IMU_READ_ERROR;
                break;
            }
            if(true == IS_BIT_SET(imu_int_status, BMA400_ASSERTED_STEP_INT)) {
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
                /// check if activity changed
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
                            /// unknown state, go to error
                            state = ACTIVITY_ERROR;
                            break;
                    }
                }
                if(true == must_update_display) {
                    update_display();
                }
            }
            break;
        case FREE_FALL:
            break;
        case EMERGENCY:
            break;
        case ACTIVITY_ERROR:
            break;
        default:
            // unknown state
            return;
    }
}