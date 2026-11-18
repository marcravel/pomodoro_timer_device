/*
 * button.c
 *
 *  Created on: Nov 18, 2026
 *      Author: ANJA RAVEL
 */
#include "button.h"

uint8_t Button_Pressed(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, Button *button)
{
    GPIO_PinState reading;

    reading = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);

    // Detect a change in the raw signal
    if (reading != button->last_read)
    {
        button->last_change = HAL_GetTick();
        button->last_read = reading;
    }

    // Wait until the signal has remained stable for 30 ms
    if ((HAL_GetTick() - button->last_change) >= 30)
    {
        if (reading != button->stable_state)
        {
            button->stable_state = reading;

            // Register only the rising edge (button press)
            if (button->stable_state == GPIO_PIN_SET)
            {
                return 1;
            }
        }
    }

    return 0;
}
