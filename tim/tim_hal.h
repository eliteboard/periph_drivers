/* Driver for Timer based on HAL functions */
#ifndef TIM_HAL_H
#define TIM_HAL_H

#include "stm32h7xx_hal.h"

struct tim_dev_s
{    
    TIM_HandleTypeDef *htim;
    int8_t (*set_prescaler) (struct tim_dev_s *self, uint16_t Prescaler);
    int8_t (*set_period) (struct tim_dev_s *self, uint16_t Period);
    int8_t (*start) (struct tim_dev_s *self);
    int8_t (*stop) (struct tim_dev_s *self);
    //TODO: consider using a vtable
};

int8_t tim_set_prescaler(struct tim_dev_s *self, uint16_t Prescaler);
int8_t tim_set_period(struct tim_dev_s *self, uint16_t Period);
int8_t tim_start(struct tim_dev_s *self);
int8_t tim_stop(struct tim_dev_s *self);

#endif