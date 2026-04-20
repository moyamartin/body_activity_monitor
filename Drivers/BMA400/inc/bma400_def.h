/**
  * @file           : bma400_def.h
  * @brief          : Header containing definitions for the bma400 driver.
  */

#ifndef BMA400_DEF_H_
#define BMA400_DEF_H_

#include <stdint.h>
#include "utils.h"
#include "bma400_interrupt.h"

/* ------------------------------------------------------------------------- */
/* Register map                                                              */
/* ------------------------------------------------------------------------- */
#define BMA400_CHIP_ID_REG              0x00
#define BMA400_ERR_REG                  0x02
#define BMA400_STATUS_REG               0x03
#define BMA400_ACC_X_LSB_REG            0x04
#define BMA400_ACC_X_MSB_REG            0x05
#define BMA400_ACC_Y_LSB_REG            0x06
#define BMA400_ACC_Y_MSB_REG            0x07
#define BMA400_ACC_Z_LSB_REG            0x08
#define BMA400_ACC_Z_MSB_REG            0x09
#define BMA400_SENSOR0_TIME0_REG        0x0A
#define BMA400_SENSOR0_TIME1_REG        0x0B
#define BMA400_SENSOR0_TIME2_REG        0x0C
#define BMA400_EVENT_REG                0x0D
#define BMA400_INT_STAT0_REG            0x0E
#define BMA400_INT_STAT1_REG            0x0F
#define BMA400_INT_STAT2_REG            0x10
#define BMA400_TEMP_DATA_REG            0x11
#define BMA400_FIFO_LENGTH0_REG         0x12
#define BMA400_FIFO_LENGTH1_REG         0x13
#define BMA400_FIFO_DATA_REG            0x14
#define BMA400_STEP_CNT0_REG            0x15
#define BMA400_STEP_CNT1_REG            0x16
#define BMA400_STEP_CNT2_REG            0x17
#define BMA400_STEP_STAT_REG            0x18
#define BMA400_ACC_CONFIG0_REG          0x19
#define BMA400_ACC_CONFIG1_REG          0x1A
#define BMA400_ACC_CONFIG2_REG          0x1B
#define BMA400_ACC_INT_CONFIG0_REG      0x1F
#define BMA400_ACC_INT_CONFIG1_REG      0x20
#define BMA400_ACC_INT1_MAP_REG         0x21
#define BMA400_ACC_INT2_MAP_REG         0x22
#define BMA400_ACC_INT12_MAP_REG        0x23
#define BMA400_INT12_IO_CTRL_REG        0x24
#define BMA400_FIFO_CONFIG0_REG         0x26
#define BMA400_FIFO_CONFIG1_REG         0x27
#define BMA400_FIFO_CONFIG2_REG         0x28
#define BMA400_FIFO_PWR_CONFIG_REG      0x29
#define BMA400_AUTOLOWPOW_0_REG         0x2A
#define BMA400_AUTOLOWPOW_1_REG         0x2B
#define BMA400_AUTOWAKEUP_0_REG         0x2C
#define BMA400_AUTOWAKEUP_1_REG         0x2D
#define BMA400_WKUP_INT_CONFIG_0_REG    0x2F
#define BMA400_WKUP_INT_CONFIG_1_REG    0x30
#define BMA400_WKUP_INT_CONFIG_2_REG    0x31
#define BMA400_WKUP_INT_CONFIG_3_REG    0x32
#define BMA400_WKUP_INT_CONFIG_4_REG    0x33
#define BMA400_ORIENTCH_CONFIG0_REG     0x35
#define BMA400_ORIENTCH_CONFIG1_REG     0x36
#define BMA400_ORIENTCH_CONFIG2_REG     0x37
#define BMA400_ORIENTCH_CONFIG3_REG     0x38
#define BMA400_ORIENTCH_CONFIG4_REG     0x39
#define BMA400_ORIENTCH_CONFIG5_REG     0x3A
#define BMA400_ORIENTCH_CONFIG6_REG     0x3B
#define BMA400_ORIENTCH_CONFIG7_REG     0x3C
#define BMA400_ORIENTCH_CONFIG8_REG     0x3D
#define BMA400_ORIENTCH_CONFIG9_REG     0x3E
#define BMA400_GEN1INT_CONFIG_REG       0x3F
#define BMA400_GEN2INT_CONFIG_REG       0x4A
#define BMA400_GENxINT_CONFIG_SIZE      11
#define BMA400_ACTH_CONFIG0_REG         0x55
#define BMA400_ACTH_CONFIG1_REG         0x56
#define BMA400_TAP_CONFIG_REG           0x57
#define BMA400_TAP_CONFIG1_REG          0x58
#define BMA400_IF_CONF_REG0_REG         0x7C
#define BMA400_SELF_TEST_REG            0x7D
#define BMA400_CMD_REG                  0x7E

/* Device chip id (read from CHIPID register) */
#define BMA400_CHIP_ID                  0x90

/* ------------------------------------------------------------------------- */
/* Available commands (CMD register 0x7E)                                    */
/* ------------------------------------------------------------------------- */
#define BMA400_CMD_NOP                  0x00
#define BMA400_CMD_FIFO_FLUSH           0xB0
#define BMA400_CMD_STEP_CNT_CLEAR       0xB1
#define BMA400_CMD_SOFT_RESET           0xB6

/* ------------------------------------------------------------------------- */
/* ERR_REG (0x02) flags                                                      */
/* ------------------------------------------------------------------------- */
#define BMA400_CMD_ERR                  BIT(1) ///< Command execution failed (clear on read)

/* ------------------------------------------------------------------------- */
/* STATUS (0x03) flags                                                       */
/* ------------------------------------------------------------------------- */
#define BMA400_DRDY_STAT                BIT(7)        ///< Data ready status
#define BMA400_CMD_RDY_STAT             BIT(4)        ///< Command ready status
#define BMA400_POWER_MODE_STAT          GENMASK(2, 1) ///< Power mode: 00 sleep, 01 low power, 10 normal
#define BMA400_INT_ACTIVE_STAT          BIT(0)        ///< 1 if any interrupt is triggered

/* ------------------------------------------------------------------------- */
/* EVENT (0x0D) flags                                                        */
/* ------------------------------------------------------------------------- */
#define BMA400_POR_DETECTED             BIT(0) ///< Power-on or soft reset detected (clear on read)

/* ------------------------------------------------------------------------- */
/* INT_STAT(1|0) (0x0E) flags                                                    */
/* ------------------------------------------------------------------------- */
#define BMA400_INT_STAT1_IENG_OVERRUN   BIT(12)         ///< Interrupt engine overrun status
#define BMA400_D_TAP_INT_STAT           BIT(11)         ///< Double tap interrupt status
#define BMA400_S_TAP_INT_STAT           BIT(10)         ///< Single tap interrupt status
#define BMA400_STEP_INT_STAT            GENMASK(9, 8)   ///< Step detector status: 0 none, 1 step, 2 double step
#define BMA400_DRDY_INT_STAT            BIT(7)          ///< Data ready interrupt status
#define BMA400_FWM_INT_STAT             BIT(6)          ///< FIFO watermark interrupt status
#define BMA400_FFULL_INT_STAT           BIT(5)          ///< FIFO full interrupt status
#define BMA400_IENG_OVERRUN_STAT        BIT(4)          ///< Interrupt engine overrun status
#define BMA400_GEN2_INT_STAT            BIT(3)          ///< Generic interrupt 2 status
#define BMA400_GEN1_INT_STAT            BIT(2)          ///< Generic interrupt 1 status
#define BMA400_ORIENTCH_INT_STAT        BIT(1)          ///< Orientation change interrupt status
#define BMA400_WKUP_INT_STAT            BIT(0)          ///< Wake-up interrupt status


/* ------------------------------------------------------------------------- */
/* INT_STAT2 (0x10) flags                                                    */
/* ------------------------------------------------------------------------- */
#define BMA400_INT_STAT2_IENG_OVERRUN   BIT(4) ///< Interrupt engine overrun status
#define BMA400_ACTCH_Z_INT_STAT         BIT(2) ///< z-axis activity change detected
#define BMA400_ACTCH_Y_INT_STAT         BIT(1) ///< y-axis activity change detected
#define BMA400_ACTCH_X_INT_STAT         BIT(0) ///< x-axis activity change detected

/* ------------------------------------------------------------------------- */
/* STEP_STAT (0x18) flags                                                    */
/* ------------------------------------------------------------------------- */
#define BMA400_STEP_ACTIVITY            GENMASK(1, 0) ///< 0 still, 1 walking, 2 running

/* ------------------------------------------------------------------------- */
/* ACC_CONFIG0 (0x19) flags                                                  */
/* ------------------------------------------------------------------------- */
#define BMA400_FILT1_BW                 BIT(7)        ///< filt1 bandwidth: 0 = 0.4*ODR, 1 = 0.2*ODR
#define BMA400_OSR_LP                   GENMASK(6, 5) ///< Oversampling ratio for low-power mode
#define BMA400_POWER_MODE_CONF          GENMASK(1, 0) ///< Power mode: 00 sleep, 01 low power, 10 normal

/* ------------------------------------------------------------------------- */
/* ACC_CONFIG1 (0x1A) flags                                                  */
/* ------------------------------------------------------------------------- */
#define BMA400_ACC_RNG                  GENMASK(7, 6) ///< Accelerometer measurement range
#define BMA400_ACC_OSR                  GENMASK(5, 4) ///< Accelerometer oversampling rate
#define BMA400_ACC_ODR                  GENMASK(3, 0) ///< Accelerometer output data rate

/* ------------------------------------------------------------------------- */
/* ACC_CONFIG2 (0x1B) flags                                                  */
/* ------------------------------------------------------------------------- */
#define BMA400_DATA_SRC                 GENMASK(3, 2) ///< Data source for accelerometer registers

/* ------------------------------------------------------------------------- */
/* INT_CONFIG0 (0x1F) flags                                                  */
/* ------------------------------------------------------------------------- */
#define BMA400_DRDY_INT_EN              BIT(7) ///< Data ready interrupt enable
#define BMA400_FWM_INT_EN               BIT(6) ///< FIFO watermark interrupt enable
#define BMA400_FFULL_INT_EN             BIT(5) ///< FIFO full interrupt enable
#define BMA400_GEN2_INT_EN_FLAG         BIT(3) ///< Generic interrupt 2 enable
#define BMA400_GEN1_INT_EN_FLAG         BIT(2) ///< Generic interrupt 1 enable
#define BMA400_ORIENTCH_INT_EN          BIT(1) ///< Orientation change interrupt enable

/* ------------------------------------------------------------------------- */
/* INT_CONFIG1 (0x20) flags                                                  */
/* ------------------------------------------------------------------------- */
#define BMA400_LATCH_INT                BIT(7) ///< 1: latched interrupt mode, 0: non-latched
#define BMA400_ACTCH_INT_EN             BIT(4) ///< Activity changed interrupt enable
#define BMA400_D_TAP_INT_EN             BIT(3) ///< Double tap interrupt enable
#define BMA400_S_TAP_INT_EN             BIT(2) ///< Single tap interrupt enable
#define BMA400_STEP_INT_EN              BIT(0) ///< Step detected interrupt enable

/* ------------------------------------------------------------------------- */
/* INT1_MAP (0x21) / INT2_MAP (0x22) flags (identical layout)                */
/* ------------------------------------------------------------------------- */
#define BMA400_DRDY_INT_MAP             BIT(7) ///< Map data ready interrupt
#define BMA400_FWM_INT_MAP              BIT(6) ///< Map FIFO watermark interrupt
#define BMA400_FFULL_INT_MAP            BIT(5) ///< Map FIFO full interrupt
#define BMA400_IENG_OVERRUN_INT_MAP     BIT(4) ///< Map interrupt engine overrun
#define BMA400_GEN2_INT_MAP             BIT(3) ///< Map generic interrupt 2
#define BMA400_GEN1_INT_MAP             BIT(2) ///< Map generic interrupt 1
#define BMA400_ORIENTCH_INT_MAP         BIT(1) ///< Map orientation change interrupt
#define BMA400_WKUP_INT_MAP             BIT(0) ///< Map wake-up interrupt

/* ------------------------------------------------------------------------- */
/* INT12_MAP (0x23) flags                                                    */
/* ------------------------------------------------------------------------- */
#define BMA400_ACTCH_INT2_MAP           BIT(7) ///< Map activity change to INT2
#define BMA400_TAP_INT2_MAP             BIT(6) ///< Map tap sensing to INT2
#define BMA400_STEP_INT2_MAP            BIT(4) ///< Map step detector to INT2
#define BMA400_ACTCH_INT1_MAP           BIT(3) ///< Map activity change to INT1
#define BMA400_TAP_INT1_MAP             BIT(2) ///< Map tap sensing to INT1
#define BMA400_STEP_INT1_MAP            BIT(0) ///< Map step detector to INT1

/* ------------------------------------------------------------------------- */
/* INT12_IO_CTRL (0x24) flags                                                */
/* ------------------------------------------------------------------------- */
#define BMA400_INT2_OUTPUT_OD           BIT(6) ///< INT2 output driver: 0 push-pull, 1 open-drain
#define BMA400_INT2_OUTPUT_LVL          BIT(5) ///< INT2 output level: 0 active low, 1 active high
#define BMA400_INT1_OUTPUT_OD           BIT(2) ///< INT1 output driver: 0 push-pull, 1 open-drain
#define BMA400_INT1_OUTPUT_LVL          BIT(1) ///< INT1 output level: 0 active low, 1 active high

/* ------------------------------------------------------------------------- */
/* FIFO_CONFIG0 (0x26) flags                                                 */
/* ------------------------------------------------------------------------- */
#define BMA400_FIFO_Z_EN                BIT(7) ///< Store z-axis samples in FIFO
#define BMA400_FIFO_Y_EN                BIT(6) ///< Store y-axis samples in FIFO
#define BMA400_FIFO_X_EN                BIT(5) ///< Store x-axis samples in FIFO
#define BMA400_FIFO_8BIT_EN             BIT(4) ///< 1: 8-bit frames, 0: 12-bit frames
#define BMA400_FIFO_DATA_SRC            BIT(3) ///< 1: acc_filt2 (100Hz), 0: acc_filt1 (variable)
#define BMA400_FIFO_TIME_EN             BIT(2) ///< Append sensor time frame when FIFO runs empty
#define BMA400_FIFO_STOP_ON_FULL        BIT(1) ///< 1: stop on full, 0: overwrite oldest (streaming)
#define BMA400_FIFO_AUTO_FLUSH          BIT(0) ///< Auto flush FIFO on power-mode change

/* ------------------------------------------------------------------------- */
/* FIFO_PWR_CONFIG (0x29) flags                                              */
/* ------------------------------------------------------------------------- */
#define BMA400_FIFO_READ_DISABLE        BIT(0) ///< Disable FIFO read power circuit

/* ------------------------------------------------------------------------- */
/* AUTOLOWPOW_1 (0x2B) flags                                                 */
/* ------------------------------------------------------------------------- */
#define BMA400_AUTO_LP_TIMEOUT_THRES_LSB    GENMASK(7, 4) ///< LSB of low-power timeout threshold
#define BMA400_AUTO_LP_TIMEOUT              GENMASK(3, 2) ///< Low-power timeout source mode
#define BMA400_AUTO_LP_GEN1_INT_TRIG        BIT(1)        ///< gen1 interrupt triggers low-power
#define BMA400_AUTO_LP_DRDY_TRIG            BIT(0)        ///< data ready triggers low-power

/* ------------------------------------------------------------------------- */
/* AUTOWAKEUP_1 (0x2D) flags                                                 */
/* ------------------------------------------------------------------------- */
#define BMA400_WAKEUP_TIMEOUT_THRES_LSB     GENMASK(7, 4) ///< LSB of wake-up timeout threshold
#define BMA400_WKUP_TIMEOUT_EN              BIT(2)        ///< Wake-up timeout source enable
#define BMA400_WKUP_INT_EN                  BIT(1)        ///< Wake-up interrupt source enable

/* ------------------------------------------------------------------------- */
/* WKUP_INT_CONFIG0 (0x2F) flags                                             */
/* ------------------------------------------------------------------------- */
#define BMA400_WKUP_Z_EN                BIT(7)        ///< Enable wake-up evaluation for z-axis
#define BMA400_WKUP_Y_EN                BIT(6)        ///< Enable wake-up evaluation for y-axis
#define BMA400_WKUP_X_EN                BIT(5)        ///< Enable wake-up evaluation for x-axis
#define BMA400_WKUP_NUM_OF_SAMPLES      GENMASK(4, 2) ///< Samples for wake-up evaluation (value+1)
#define BMA400_WKUP_REFU                GENMASK(1, 0) ///< Wake-up reference update mode

/* ------------------------------------------------------------------------- */
/* ORIENTCH_CONFIG0 (0x35) flags                                             */
/* ------------------------------------------------------------------------- */
#define BMA400_ORIENT_Z_EN              BIT(7)        ///< Enable orientation change evaluation for z-axis
#define BMA400_ORIENT_Y_EN              BIT(6)        ///< Enable orientation change evaluation for y-axis
#define BMA400_ORIENT_X_EN              BIT(5)        ///< Enable orientation change evaluation for x-axis
#define BMA400_ORIENT_DATA_SRC          BIT(4)        ///< 0: acc_filt2, 1: acc_filt_lp
#define BMA400_ORIENT_REF_UPDATE        GENMASK(3, 2) ///< Reference update mode
#define BMA400_ORIENT_STABILITY_MODE    GENMASK(1, 0) ///< Stability check mode

/* ------------------------------------------------------------------------- */
/* GEN(1|2)INT_CONFIG0 (0x3F / 0x4A) flags                                   */
/* ------------------------------------------------------------------------- */
#define BMA400_GEN_ACT_Z_EN             BIT(7)        ///< Evaluate z-axis for generic interrupt
#define BMA400_GEN_ACT_Y_EN             BIT(6)        ///< Evaluate y-axis for generic interrupt
#define BMA400_GEN_ACT_X_EN             BIT(5)        ///< Evaluate x-axis for generic interrupt
#define BMA400_GEN_DATA_SRC             BIT(4)        ///< 0: acc_filt1, 1: acc_filt2
#define BMA400_GEN_ACT_REF_UPDATE       GENMASK(3, 2) ///< Reference update mode
#define BMA400_GEN_ACT_HYST             GENMASK(1, 0) ///< Hysteresis configuration

/* ------------------------------------------------------------------------- */
/* GEN(1|2)INT_CONFIG1 (0x40 / 0x4B) flags                                   */
/* ------------------------------------------------------------------------- */
#define BMA400_GEN_CRITERION_SEL        BIT(1) ///< 0: acc below threshold, 1: acc above threshold
#define BMA400_GEN_COMB_SEL             BIT(0) ///< 0: OR, 1: AND combination of enabled axes

/* ------------------------------------------------------------------------- */
/* ACTCH_CONFIG1 (0x56) flags                                                */
/* ------------------------------------------------------------------------- */
#define BMA400_ACTCH_Z_EN               BIT(7)        ///< Evaluate z-axis for activity change
#define BMA400_ACTCH_Y_EN               BIT(6)        ///< Evaluate y-axis for activity change
#define BMA400_ACTCH_X_EN               BIT(5)        ///< Evaluate x-axis for activity change
#define BMA400_ACTCH_DATA_SRC           BIT(4)        ///< 0: acc_filt1, 1: acc_filt2
#define BMA400_ACTCH_NPTS               GENMASK(3, 0) ///< Sample count code (32/64/128/256/512)

/* ------------------------------------------------------------------------- */
/* TAP_CONFIG (0x57) flags                                                   */
/* ------------------------------------------------------------------------- */
#define BMA400_SEL_AXIS                 GENMASK(4, 3) ///< Axis used for tap evaluation
#define BMA400_TAP_SENSITIVITY          GENMASK(2, 0) ///< Tap sensitivity

/* ------------------------------------------------------------------------- */
/* TAP_CONFIG1 (0x58) flags                                                  */
/* ------------------------------------------------------------------------- */
#define BMA400_QUIET_DT                 GENMASK(5, 4) ///< Minimum quiet time between double-tap peaks
#define BMA400_QUIET                    GENMASK(3, 2) ///< Minimum quiet time before/after tap
#define BMA400_TICS_TH                  GENMASK(1, 0) ///< Max time between upper/lower tap peak

/* ------------------------------------------------------------------------- */
/* IF_CONF (0x7C) flags                                                      */
/* ------------------------------------------------------------------------- */
#define BMA400_SPI3_EN                  BIT(0) ///< 0: SPI 4-wire, 1: SPI 3-wire

/* ------------------------------------------------------------------------- */
/* self_test (0x7d) flags                                                    */
/* ------------------------------------------------------------------------- */
#define BMA400_ACC_SELF_TEST_SIGN       BIT(3) ///< 0: negative excitation, 1: positive
#define BMA400_ACC_SELF_TEST_EN_Z       BIT(2) ///< Trigger self-test for z-axis
#define BMA400_ACC_SELF_TEST_EN_Y       BIT(1) ///< Trigger self-test for y-axis
#define BMA400_ACC_SELF_TEST_EN_X       BIT(0) ///< Trigger self-test for x-axis

/* ------------------------------------------------------------------------- */
/* Shared axis-enable helpers (same bit positions across ACTCH, GENxINT,     */
/* ORIENTCH, WKUP_INT configuration registers)                               */
/* ------------------------------------------------------------------------- */
#define BMA400_AXIS_X_EN                BIT(5)
#define BMA400_AXIS_Y_EN                BIT(6)
#define BMA400_AXIS_Z_EN                BIT(7)

/* ------------------------------------------------------------------------- */
/* System delays                                                             */
/* ------------------------------------------------------------------------- */
#define BMA400_SOFT_RESET_DELAY 5000U


/* ------------------------------------------------------------------------- */
/* Enumerations                                                              */
/* ------------------------------------------------------------------------- */

/**
 * @brief Enum representing number of samples for interrupt condition evaluation
 */
typedef enum {
    BMA400_WKUP_NUM_SAMPLES_0 = 0,
    BMA400_WKUP_NUM_SAMPLES_1,
    BMA400_WKUP_NUM_SAMPLES_2,
    BMA400_WKUP_NUM_SAMPLES_3,
    BMA400_WKUP_NUM_SAMPLES_4,
    BMA400_WKUP_NUM_SAMPLES_5,
    BMA400_WKUP_NUM_SAMPLES_6,
    BMA400_WKUP_NUM_SAMPLES_7,
} bma400_wkup_int_samples_t;

/**
 * @brief Enum represeting the accelerometer filter selection
 */
typedef enum {
    BMA400_ACC_FILT1 = 0,   ///< Variable ODR filter
    BMA400_ACC_FILT2,       ///< Fixed 100Hz output data rate filter
    BMA400_ACC_FILT_LP,     ///< Fixed 100Hz output data rate filter, 1Hz bandwidth
} bma400_acc_filter_t;


/**
 * @brief Enum represeting the accelerometer range selection
 */
typedef enum {
    BMA400_RNG_2G = 0,  ///< Selected g-range 2g
    BMA400_RNG_4G,      ///< Selected g-range 4g
    BMA400_RNG_8G,      ///< Selected g-range 8g
    BMA400_RNG_16G,     ///< Selected g-range 16g
} bma400_acc_rng_t;

/**
 * @brief Enum representing the output data rate of the accelerometer
 */
typedef enum {
    BMA400_ODR_12_5HZ = 0x05,   ///< 12.5Hz
    BMA400_ODR_25HZ,            ///< 25Hz
    BMA400_ODR_50HZ,            ///< 50Hz
    BMA400_ODR_100HZ,           ///< 100Hz
    BMA400_ODR_200HZ,           ///< 200Hz
    BMA400_ODR_400HZ,           ///< 400Hz
    BMA400_ODR_800HZ,           ///< 800Hz
} bma400_acc_odr_t;


/**
 * @brief Enum representing the acceloremeter power mode
 */
typedef enum {
    BMA400_SLEEP = 0,
    BMA400_LOW_POWER,
    BMA400_NORMAL,
} bma400_power_mode_t;

/**
 * @brief Enum representing the activity the user is doing detected from the accelerometer
 */
typedef enum {
    BMA400_STILL = 0,
    BMA400_WALKING,
    BMA400_RUNNING,
} bma400_activity_t;

/**
 * @brief Enum representing possible errors from the BMA400
 */
typedef enum {
    BMA400_OK = 0,
    BMA400_INVAL,
    BMA400_WRITE_ERROR,
    BMA400_READ_ERROR,
    BMA400_BUSY,
    BMA400_CMD_EXEC_FAILED, ///< ERR_REG.cmd_err == 1
} bma400_error_t;

/**
 * @brief Sensor / feature selection for set/get parameter APIs.
 *
 * Prefixed with BMA400_SENSOR_ to avoid collision with BIT-macro names used
 * for the interrupt enable/status flags (e.g. BMA400_GEN1_INT_MAP).
 */
typedef enum {
    BMA400_SENSOR_ACCEL = 0,
    BMA400_SENSOR_TAP,
    BMA400_SENSOR_ACT_CHANGE,
    BMA400_SENSOR_GEN1,
    BMA400_SENSOR_GEN2,
    BMA400_SENSOR_ORIENT_CHANGE,
    BMA400_SENSOR_STEP_COUNTER,
} bma400_sensor_t;

/**
 * @brief Tap sensitivity (TAP_CONFIG.tap_sensitivity).
 *
 * 0 is the most sensitive, 7 the least sensitive.
 */
typedef enum {
    BMA400_TAP_SENSITIVITY_0 = 0,
    BMA400_TAP_SENSITIVITY_1,
    BMA400_TAP_SENSITIVITY_2,
    BMA400_TAP_SENSITIVITY_3,
    BMA400_TAP_SENSITIVITY_4,
    BMA400_TAP_SENSITIVITY_5,
    BMA400_TAP_SENSITIVITY_6,
    BMA400_TAP_SENSITIVITY_7,
} bma400_tap_sensitivity_t;

/**
 * @brief Interrupt pin routing selection.
 */
typedef enum {
    BMA400_UNMAP_INT_PIN = 0,
    BMA400_INT_CHANNEL_1,
    BMA400_INT_CHANNEL_2,
    BMA400_MAP_BOTH_INT_PINS,
} bma400_int_chan_t;

/**
 * @brief Axis used by the tap algorithm (TAP_CONFIG.sel_axis).
 */
typedef enum {
    BMA400_Z_AXIS = 0,
    BMA400_Y_AXIS,
    BMA400_X_AXIS,
} bma400_tap_axis_t;

/**
 * @brief TAP_CONFIG1.tics_th (max time between upper and lower tap peak).
 */
typedef enum {
    BMA400_TICS_TH_6_DS = 0, ///< 6 data samples
    BMA400_TICS_TH_9_DS,     ///< 9 data samples
    BMA400_TICS_TH_12_DS,    ///< 12 data samples
    BMA400_TICS_TH_18_DS,    ///< 18 data samples
} bma400_tap_tics_th_t;

/**
 * @brief TAP_CONFIG1.quiet — quiet time before/after a tap.
 */
typedef enum {
    BMA400_QUIET_60_DS = 0, ///< 60 samples
    BMA400_QUIET_80_DS,     ///< 80 samples
    BMA400_QUIET_100_DS,    ///< 100 samples
    BMA400_QUIET_120_DS,    ///< 120 samples
} bma400_tap_quiet_t;

/**
 * @brief TAP_CONFIG1.quiet_dt — minimum time between the two taps of a double tap.
 */
typedef enum {
    BMA400_QUIET_DT_4_DS = 0, ///< 4 samples
    BMA400_QUIET_DT_8_DS,     ///< 8 samples
    BMA400_QUIET_DT_12_DS,    ///< 12 samples
    BMA400_QUIET_DT_16_DS,    ///< 16 samples
} bma400_tap_quiet_dt_t;

/**
 * @brief ACTCH_CONFIG1.actch_npts — sample count used for activity change evaluation.
 */
typedef enum {
    BMA400_NPTS_32 = 0,   ///< 32 points
    BMA400_NPTS_64,       ///< 64 points
    BMA400_NPTS_128,      ///< 128 points
    BMA400_NPTS_256,      ///< 256 points
    BMA400_NPTS_512,      ///< 512 points
} bma400_ach_npts_t;

/**
 * @brief GEN(1|2)INT_CONFIG0.act_refu — reference update mode for generic interrupts.
 */
typedef enum {
    BMA400_GEN_ACT_REFU_MANUAL = 0,     ///< Manual update
    BMA400_GEN_ACT_REFU_ONE_TIME,       ///< One-time automated update using selected data source
    BMA400_GEN_ACT_REFU_EVERYTIME,      ///< Every-time automated update using selected data source
    BMA400_GEN_ACT_REFU_EVERYTIME_LP,   ///< Every-time automated update using acc_filt_lp
} bma400_gen_act_ref_update_t;

/**
 * @brief GEN(1|2)INT_CONFIG0.act_hyst — hysteresis configuration.
 */
typedef enum {
    BMA400_GEN_ACT_HYST_NOT_ACTIVE = 0, ///< No hysteresis
    BMA400_GEN_ACT_HYST_24MG,           ///< 24 mg
    BMA400_GEN_ACT_HYST_48MG,           ///< 48 mg
    BMA400_GEN_ACT_HYST_96MG,           ///< 96 mg
} bma400_act_hyst_conf_t;

/**
 * @brief ORIENTCH_CONFIG0.orient_refu — reference update mode for orientation change.
 */
typedef enum {
    BMA400_ORIENT_REF_UPDATE_MANUAL = 0,    ///< Manual update
    BMA400_ORIENT_REF_UPDATE_ONETIME_2,     ///< One-time automated update using acc_filt2
    BMA400_ORIENT_REF_UPDATE_ONETIME_LP,    ///< One-time automated update using acc_filt_lp
} bma400_orient_ref_update_t;

/**
 * @brief ORIENTCH_CONFIG0.stability_mode — stability check mode.
 */
typedef enum {
    BMA400_STABILITY_MODE_INACTIVE = 0, ///< Stability check disabled
    BMA400_STABILITY_MODE_FILT2,        ///< acc_filt2 used for stability check
    BMA400_STABILITY_MODE_LP,           ///< Low-pass filtered acceleration used for stability check
} bma400_stability_mode_t;

/**
 * @brief WKUP_INT_CONFIG0.wkup_refu — wake-up reference update mode.
 */
typedef enum {
    BMA400_WKUP_REFU_MANUAL = 0,   ///< Manual update
    BMA400_WKUP_REFU_ONETIME,      ///< One-time automated update before low-power
    BMA400_WKUP_REFU_EVERYTIME,    ///< Every time after data conversion
} bma400_wkup_ref_update_t;

/**
 * @brief AUTOLOWPOW_1.auto_lp_timeout — auto-low-power timeout source.
 */
typedef enum {
    BMA400_AUTO_LP_TIMEOUT_DISABLED = 0, ///< Timeout disabled
    BMA400_AUTO_LP_TIMEOUT_ON_EXPIRE,    ///< Enter low power on timeout expire
    BMA400_AUTO_LP_TIMEOUT_RESET_ON_G2,  ///< As above, but timer resets if gen2_int fires
    BMA400_AUTO_LP_TIMEOUT_ON_EXPIRE_2,  ///< Same as ON_EXPIRE
} bma400_auto_lp_timeout_t;

/**
 * @brief ACC_CONFIG0.filt1_bw — filt1 output bandwidth selection.
 */
typedef enum {
    BMA400_FILT1_BW_HIGH = 0, ///< 0.4 * ODR (default)
    BMA400_FILT1_BW_LOW,      ///< 0.2 * ODR
} bma400_filt1_bw_t;

/* ------------------------------------------------------------------------- */
/* Parameter / configuration structures                                      */
/* ------------------------------------------------------------------------- */

/**
 * @brief Accelerometer configuration
 */
typedef struct {
    bma400_acc_odr_t odr;           ///< Output data rate
    bma400_acc_rng_t range;         ///< Measurement range
    bma400_acc_filter_t data_src;   ///< Data source filter selection
    uint8_t osr;                    ///< Oversampling rate for the data source
    uint8_t osr_lp;                 ///< Oversampling setting for low-power mode
    uint8_t filt1_bw;               ///< Filter 1 bandwidth (@ref bma400_filt1_bw_t)
    bma400_int_chan_t int_chan;     ///< Interrupt channel routing
} bma400_acc_conf_t;

/**
 * @brief Tap interrupt configuration
 */
typedef struct {
    bma400_tap_axis_t axes_sel;             ///< Axis used for tap detection
    bma400_tap_sensitivity_t sensitivity;   ///< Tap sensitivity
    bma400_tap_tics_th_t tics_th;           ///< Max time between upper/lower tap peak
    bma400_tap_quiet_t quiet;               ///< Quiet time before/after a tap
    bma400_tap_quiet_dt_t quiet_dt;         ///< Min time between double-tap peaks
    bma400_int_chan_t int_chan;             ///< Interrupt channel routing
} bma400_tap_conf_t;

/**
 * @brief Activity change interrupt configuration
 */
typedef struct {
    uint8_t act_ch_thres;           ///< Activity-change threshold (1 LSB = 8 mg)
    uint8_t axes_sel;               ///< OR'd combination of BMA400_AXIS_[X|Y|Z]_EN
    uint8_t data_source;            ///< 0: acc_filt1, 1: acc_filt2 (see BMA400_ACTCH_DATA_SRC)
    bma400_ach_npts_t act_ch_ntps;  ///< Sample count code
    bma400_int_chan_t int_chan;     ///< Interrupt channel routing
} bma400_act_ch_conf_t;

/**
 * @brief Generic interrupt (gen1 / gen2) configuration
 */
typedef struct {
    uint8_t gen_int_thres;                  ///< Threshold (1 LSB = 8 mg)
    uint16_t gen_int_dur;                   ///< Duration (in data samples) the condition must hold
    uint8_t axes_sel;                       ///< OR'd combination of BMA400_AXIS_[X|Y|Z]_EN
    uint8_t data_src;                       ///< 0: acc_filt1, 1: acc_filt2
    uint8_t criterion_sel;                  ///< 0: below threshold, 1: above threshold
    uint8_t evaluate_axes;                  ///< 0: OR, 1: AND combination of enabled axes
    bma400_gen_act_ref_update_t ref_update; ///< Reference update mode
    bma400_act_hyst_conf_t hysteresis;      ///< Hysteresis configuration
    uint16_t int_thres_ref_x;               ///< Reference value for x axis (12-bit, signed)
    uint16_t int_thres_ref_y;               ///< Reference value for y axis (12-bit, signed)
    uint16_t int_thres_ref_z;               ///< Reference value for z axis (12-bit, signed)
    bma400_int_chan_t int_chan;             ///< Interrupt channel routing
} bma400_gen_int_conf_t;

/**
 * @brief Orientation change interrupt configuration
 */
typedef struct {
    uint8_t axes_sel;                       ///< OR'd combination of BMA400_AXIS_[X|Y|Z]_EN
    uint8_t data_src;                       ///< 0: acc_filt2, 1: acc_filt_lp
    bma400_orient_ref_update_t ref_update;  ///< Reference update mode
    bma400_stability_mode_t stability_mode; ///< Stability check mode
    uint8_t orient_thres;                   ///< Threshold (1 LSB = 8 mg)
    uint8_t stability_thres;                ///< Stability threshold (1 LSB = 8 mg)
    uint8_t orient_int_dur;                 ///< Duration (10 ms / LSB) new orientation must hold
    uint16_t orient_ref_x;                  ///< x-axis reference (12-bit, signed)
    uint16_t orient_ref_y;                  ///< y-axis reference (12-bit, signed)
    uint16_t orient_ref_z;                  ///< z-axis reference (12-bit, signed)
    bma400_int_chan_t int_chan;             ///< Interrupt channel routing
} bma400_orient_int_conf_t;

/**
 * @brief Step counter / step detector configuration
 *
 * The BMA400 does not expose configurable step-detection thresholds over the
 * register interface; only the routing of the resulting interrupt is user
 * configurable.
 */
typedef struct {
    bma400_int_chan_t int_chan;     ///< Interrupt channel routing
} bma400_step_int_conf_t;

/**
 * @brief Auto wakeup configuration
 */
typedef struct {
    uint8_t enable_wakeup;  ///< Use wake-up interrupt for auto-wake-up. Assignable macro is BMA400_WKUP_INT_EN
    uint8_t wakeup_timeout; ///< Enable auto wake-up by using timeout. Assignable macro is BMA400_WKUP_TIMEOUT_EN

    /* Timeout threshold after which auto wake-up occurs
     * It is 12bit value configurable at 2.5ms/LSB
     * Maximum timeout is 10.24s (4096 * 2.5) for
     * which the assignable macro is :
     *      - BMA400_AUTO_WAKEUP_TIMEOUT_MAX
     */
    uint16_t timeout_thres;
} bma400_auto_wakeup_conf_t;

/**
 * @brief Wakeup configuration
 */
typedef struct {
    bma400_wkup_ref_update_t ref_update;    ///< Wakeup reference update. @ref bma400_wkup_ref_update_t
    bma400_wkup_int_samples_t sample_count; ///< Number of samples for interrupt condition evaluation. @ref bma400_wkup_int_samples_t
    uint8_t wakeup_axes_en;                 ///< Enable low power wake-up interrupt.  OR'd combination of BMA400_AXIS_[X|Y|Z]_EN
    uint8_t int_wkup_threshold;             ///< Interrupt threshold configuration
    uint8_t int_wkup_ref_x;                 ///< Reference acceleration x-axis for the wake-up interrupt
    uint8_t int_wkup_ref_y;                 ///< Reference acceleration y-axis for the wake-up interrupt
    uint8_t int_wkup_ref_z;                 ///< Reference acceleration z-axis for the wake-up interrupt
    bma400_int_chan_t int_chan;
} bma400_wakeup_conf_t;

/**
 * @brief Auto low-power configurations
 */
typedef struct {
    /* Enable auto low power mode using  data ready interrupt /
     * Genric interrupt1 / timeout counter value
     * Assignable macros :
     * - BMA400_AUTO_LP_DRDY_TRIGGER
     * - BMA400_AUTO_LP_GEN1_TRIGGER
     */
    uint8_t auto_low_power_trigger;                     ///< Enable auto low power mode using data ready interrupt / generic interrupt. Assignable mmacros BMA400_AUTO_LP_GEN!_INT_TRIG or BMA400_AUTO_LP_DRDY_TRIG
    bma400_auto_lp_timeout_t auto_low_power_timeout;    ///< Auto low power timeout as source for auto-low-power condition.
    uint16_t auto_lp_timeout_threshold;                 ///< Timeout threshold after which auto wake-up occurs.
} bma400_auto_lp_conf_t;

/**
 * @brief FIFO configurations
 */
typedef struct bma400_fifo_conf
{
    /* Select FIFO configurations to enable/disable
     * Assignable Macros :
     *   - BMA400_FIFO_AUTO_FLUSH
     *   - BMA400_FIFO_STOP_ON_FULL
     *   - BMA400_FIFO_TIME_EN
     *   - BMA400_FIFO_DATA_SRC
     *   - BMA400_FIFO_8_BIT_EN
     *   - BMA400_FIFO_X_EN
     *   - BMA400_FIFO_Y_EN
     *   - BMA400_FIFO_Z_EN
     */
    uint8_t conf_regs;

    /* Enable/ disable selected FIFO configurations
     * Assignable Macros :
     *   - BMA400_ENABLE
     *   - BMA400_DISABLE
     */
    uint8_t conf_status;
    uint16_t fifo_watermark;             ///< Value to set the water-mark
    bma400_int_chan_t fifo_full_channel; ///< Interrupt pin mapping for FIFO full interrupt
    bma400_int_chan_t fifo_wm_channel;   ///< Interrupt pin mapping for FIFO water-mark interrupt
} bma400_fifo_conf_t;

/*
 * enum to select device settings
 */
typedef enum {
    BMA400_AUTOWAKEUP_TIMEOUT,
    BMA400_AUTOWAKEUP_INT,
    BMA400_AUTO_LOW_POWER,
    BMA400_INT_PIN_CONF,
    BMA400_INT_OVERRUN_CONF,
    BMA400_FIFO_CONF
} bma400_device_t;

typedef struct {
    bma400_int_chan_t int_chan;
} bma400_int_overrun_t;

/**
 * @brief Union of all per-feature parameter blocks.
 */
typedef union {
    bma400_acc_conf_t accel;
    bma400_tap_conf_t tap;
    bma400_act_ch_conf_t act_ch;
    bma400_gen_int_conf_t gen_int;
    bma400_orient_int_conf_t orient;
    bma400_step_int_conf_t step_cnt;
} bma400_set_param_t;

/**
 * @brief Union of all device configuration parameters.
 */
typedef union {
    bma400_auto_wakeup_conf_t auto_wakeup;  ///< Auto wakeup configurations
    bma400_wakeup_conf_t wakeup;            ///< Wakeup configurations 
    bma400_auto_lp_conf_t auto_lp;          ///< Auto low power configurations
    bma400_interrupt_t int1_conf;           ///< Interrupt pin 1 configuration
    bma400_interrupt_t int2_conf;           ///< Interrupt pin 1 configuration
    bma400_fifo_conf_t fifo_conf;           ///< FIFO configuration
    bma400_int_overrun_t overrun_int;       ///< Interrupt overrun configuration
} bma400_device_params_t;

/**
 * @brief BMA400 Device configuration
 */
typedef struct {
    bma400_device_t type;           ///< Device feature selection
    bma400_device_params_t param;   ///< Device feature configuration
} bma400_device_cfg_t;


/**
 * @brief Sensor selection and its configuration
 */
typedef struct {
    bma400_sensor_t type;       ///< Sensor / feature to configure
    bma400_set_param_t param;   ///< Feature-specific configuration
} bma400_sensor_cfg_t;

/**
 * @brief FIFO frame container
 */
typedef struct {
    uint8_t *data;                  ///< User-provided buffer for FIFO bytes
    uint16_t length;                ///< In: buffer size / bytes to read. Out: bytes read
    uint8_t fifo_time_enable;       ///< FIFO time frame enable
    uint8_t fifo_8_bit_en;          ///< FIFO 8-bit mode enable
    uint8_t fifo_data_enable;       ///< OR'd BMA400_FIFO_[X|Y|Z]_EN
    uint16_t accel_byte_start_idx;  ///< Parser bookkeeping: set to length when fully parsed
    uint8_t conf_change;            ///< Configuration change detected during FIFO read
    uint32_t fifo_sensor_time;      ///< Last sensor time value parsed from FIFO
} bma400_fifo_data_t;

/**
 * @brief Interrupt enable/disable entry passed to bma400_enable_interrupt().
 */
typedef struct {
    bma400_int_type_t type; ///< Interrupt source to enable or disable
    uint8_t conf;           ///< Non-zero enables the interrupt, zero disables it
} bma400_int_enable_t;


/**
 * @brief BMA400 device descriptor
 */
typedef struct {
    uint8_t chip_id;                    ///< Cached chip ID read from the sensor
    void *intf_ptr;                     ///< Pointer to the underlying port structure
    bma400_interrupt_t *int1_config;    ///< Optional INT1 pin configuration (NULL if unused)
    bma400_interrupt_t *int2_config;    ///< Optional INT2 pin configuration (NULL if unused)
} bma400_dev_t;

#endif /* BMA400_DEF_H_ */
