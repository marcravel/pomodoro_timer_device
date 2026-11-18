/*
 * button.h
 *
 *  Created on: Nov 18, 2026
 *      Author: ANJA RAVEL
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stm32f1xx_hal.h>

/* Button debouncing */
typedef struct
{
    GPIO_PinState last_read;
    GPIO_PinState stable_state;
    uint32_t last_change;
} Button;

// Prototypes
uint8_t Button_Pressed(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, Button *button);

#endif /* INC_BUTTON_H_ */
