/**
  * @file           : API_delay.h
  * @brief          : Header for API_delay.c file.
  *                   This file contains the common defines of delay library.
  */

#ifndef API_DELAY_H_
#define API_DELAY_H_

#include <stdint.h> ///< include standard integer types
#include <stdbool.h> ///< include standard boolean type


typedef uint32_t tick_t; ///< alias for tick type
typedef bool bool_t; ///< alias for boolean type

/**
 * @brief structure that represents a non-blocking delay
 */
typedef struct {
    tick_t startTime; ///< variable to store the start time of the delay
    tick_t duration; ///< variable to store the duration of the delay
    bool_t running; ///< variable to indicate if the delay is currently running
} delay_t;

/**
 * @brief Initializes a delay structure with the specified duration.
 *
 * @param[in] delay pointer to the delay structure to be initialized
 * @param[in] duration the duration of the delay in ticks
 * @note: This function does not starts the delay, it only initializes the
 * structure. To start the delay, you need to call the delayRead function after
 * initialization.
 */
void delayInit(delay_t *delay, tick_t duration);

/**
 * @brief Checks if the specified delay has elapsed.
 *
 * @param[in] delay pointer to the delay structure to be checked
 * @return true if the delay has elapsed, false otherwise
 * @note it starts the delay if it is not running, so you can use this function
 * to both start and check the delay.
 */
bool_t delayRead(delay_t *delay);


/**
 * @brief Modifies duration time of a delay
 *
 * @param[in] delay pointer to the delay structure to be modified
 * @param[in] duration the new duration of the delay in ticks
 */
void delayWrite(delay_t *delay, tick_t duration);


/**
 * @brief Verifies if the delay is running
 *
 * @param[in] delay pointer to the delay structure to be modified
 * @return true if delay is running, otherwise false
 */
bool_t delayIsRunning(const delay_t *delay);

#endif /* API_DELAY_H_ */
