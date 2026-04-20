/**
  * @file           : activity_monitor.h
  * @brief          : Header for activity_monitor.c file.
  *                   This file contains the common defines of the activity
  *                   monitor FSM that orchestrates UART, display, IMU and
  *                   button inputs to track the user's activity.
  */
#ifndef ACTIVITY_MONITOR_H_
#define ACTIVITY_MONITOR_H_

/**
 * @brief Error codes reported by the activity monitor module
 */
typedef enum {
    AM_OK = 0,                  ///< No error
    AM_UART_INIT_ERROR,         ///< UART peripheral initialization failed
    AM_UART_WRITE_ERROR,        ///< Writing to UART failed
    AM_DISPLAY_INIT_ERROR,      ///< LCD1602 display initialization failed
    AM_DISPLAY_WRITE_ERROR,     ///< Writing to the LCD1602 display failed
    AM_IMU_INIT_ERROR,          ///< BMA400 IMU initialization failed
    AM_IMU_CONFIG_ERROR,        ///< BMA400 sensor/interrupt configuration failed
    AM_IMU_READ_ERROR,          ///< Reading from the BMA400 IMU failed
} activity_monitor_error_t;

/**
 * @brief States of the activity monitor FSM
 */
typedef enum {
    STILL,                      ///< User is not moving
    WALKING,                    ///< IMU reports walking activity
    RUNNING,                    ///< IMU reports running activity
    FREE_FALL,                  ///< IMU detected a free-fall event
    EMERGENCY,                  ///< Emergency condition triggered (button or free-fall timeout)
    ACTIVITY_ERROR,             ///< FSM entered an error state; see last reported error
} activity_monitor_state_t;


/**
 * @brief Load initial state and initialize peripherals
 * 
 * This function initializes the activity monitor FSM by setting the initial state
 * to STILL and configuring all the necessary peripherals.
 * It should be called once at the start of the program.
 */
activity_monitor_error_t activity_monitor_init(void);

/**
 * @brief update FSM according to inputs
 * 
 * This function should be called periodically in the main loop to update the
 * state of the FSM based on the BMA400 and button inputs. It checks the current
 * state and transitions to the next state accordingly.
 */
void activity_monitor_update(void);


#endif /* ACTIVITY_MONITOR_H_ */