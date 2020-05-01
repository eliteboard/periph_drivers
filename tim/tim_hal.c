/* Driver for Timer based on HAL functions */

#include <errno.h>
#include "stm32h7xx_hal.h"
#include "tim_hal.h"

int8_t tim_set_prescaler(struct tim_dev_s *self, uint16_t Prescaler)
{
    //self->htim.Init.Prescaler = Prescaler;
    self->htim->Instance->PSC = Prescaler;
}

int8_t tim_set_period(struct tim_dev_s *self, uint16_t Period)
{
    //self->htim.Init.Period = Period;
    self->htim->Instance->ARR = Period; //Autoreload register
}

int8_t tim_start(struct tim_dev_s *self)
{    
    if(HAL_TIM_Base_Start_IT(self->htim) != HAL_OK)
    {
        /* Start Error */
        Error_Handler();
    }
}

int8_t tim_stop(struct tim_dev_s *self)
{
    if(HAL_TIM_Base_Stop_IT(self->htim) != HAL_OK)
    {
        /* Stop Error */
        Error_Handler();
    }
}