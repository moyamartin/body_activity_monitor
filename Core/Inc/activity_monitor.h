/**
  * @file           : API_debounce.h
  * @brief          : Header for activity_monitor.c file.
  *                   This file contains the common defines of the activity monitor FSM.
  */
#ifndef ACTIIVITY_MONITOR_H_
#define ACTIVITY_MONITOR_H_

/**
 * @brief Enum representation of activityMonitor errors
 */
typedef enum {
    AM_OK = 0,
    AM_UART_INIT_ERROR,
    AM_UART_WRITE_ERROR,
    AM_DISPLAY_INIT_ERROR,
    AM_DISPLAY_WRITE_ERROR,
    AM_IMU_INIT_ERROR,
    AM_IMU_CONFIG_ERROR,
    AM_IMU_READ_ERROR,
} activity_monitor_error_t;

/**
 * @brief Representation of each state of the activity monitor FSM
 */
typedef enum {
    STILL,
    WALKING,
    RUNNING,
    FREE_FALL,
    EMERGENCY,
    ACTIVITY_ERROR,
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