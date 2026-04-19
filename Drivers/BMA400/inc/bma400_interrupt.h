#ifndef BMA400_INTERRUPT_H_
#define BMA400_INTERRUPT_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BMA400_DATA_READY_INT_EN,       ///< DRDY Interrupt (renamed to avoid collision with the BMA400_DRDY_INT_EN bit mask)
    BMA400_FIFO_WM_INT_EN,          ///< FIFO watermark interrupt
    BMA400_FIFO_FULL_INT_EN,        ///< FIFO full interrupt
    BMA400_GEN2_INT_EN,             ///< Generic interrupt 2
    BMA400_GEN1_INT_EN,             ///< Generic interrupt 1
    BMA400_ORIENT_CHANGE_INT_EN,    ///< Orient change interrupt
    BMA400_LATCH_INT_EN,            ///< Latch interrupt
    BMA400_ACTIVITY_CHANGE_INT_EN,  ///< Activity change interrupt
    BMA400_DOUBLE_TAP_INT_EN,       ///< Double tap interrupt
    BMA400_SINGLE_TAP_INT_EN,       ///< Single tap interrupt
    BMA400_STEP_COUNTER_INT_EN,     ///< Step counter interrupt
    BMA400_AUTO_WAKEUP_EN,          ///< Auto wakeup interrupt
} bma400_int_type_t;


typedef struct {
    void *port;                     ///< GPIO port (e.g. GPIO_TypeDef * on STM32 HAL)
    uint16_t pin;                   ///< GPIO pin mask (GPIO_PIN_x on STM32 HAL)
    bool active_high;               ///< true if interrupt is active high, false if active low
    bool open_drain;                ///< true if output is open-drain, false if push-pull
    bool triggered;                 ///< Flag to indicate if the interrupt was triggered, it must be updated in the GPIO's callback
    bma400_int_type_t type;         ///< type of interrupt
} bma400_interrupt_t;


#endif /* BMA400_INTERRUPT_H_ */
