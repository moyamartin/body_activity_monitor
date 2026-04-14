/**
  * @file           : API_debounce.h
  * @brief          : Header for API_debounce.c file.
  *                   This file contains the common defines of debounce library.
  */
#ifndef API_DEBOUNCE_H_
#define API_DEBOUNCE_H_

#include "API_delay.h"

/**
 * @brief representation of each state of the FSM
 */
typedef enum {
	BUTTON_UP,			///< The button is up after 40mS of rising edge detected
	BUTTON_FALLING,		///< A falling edge has been detected
	BUTTON_DOWN,		///< delay of 40mS passed and logic level of button is low, FSM passed to BUTTON_DOWN
	BUTTON_RISING		///< A rising edge has been detected, waiting for delay 40mS and button up so FSM transitions to BUTTON_UP
} debounceState_t;

/**
 * @brief load initial state
 * 
 * This function initializes the debounce FSM by setting the initial state to
 * BUTTON_UP and initializing the debounce delay timer.
 * It should be called once at the start of the program.
 */
void debounceFSM_init();

/**
 * @brief update FSM according to inputs
 * 
 * This function should be called periodically in the main loop to update the
 * state of the FSM based on the button input and timing. It checks the current
 * state and transitions to the next state based on the button's logic level and
 * the debounce delay.
 */
void debounceFSM_update();

/**
 * @brief public function to get key value
 *
 * @return true if the button was pressed, false otherwise
 * @note this function will return true only once per button press, it resets
 * the keyPressed variable after reading it
 */
bool_t readKey();

#endif /* API_DEBOUNCE_H_ */
