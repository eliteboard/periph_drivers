/* Driver for internal ADC */

#ifndef INT_ADC_H
#define INT_ADC_H

#include <stdint.h>
#include "stm32h7xx_hal.h"

#define INT_ADC_BUFFER_LENGTH (4096)

struct int_adc_dev_s
{    
    ADC_HandleTypeDef *hadc;
    struct tim_dev_s *tim_dev;
    int8_t (*arm) (struct int_adc_dev_s *self);  //arms ADC
    uint32_t* (*get_data) (struct int_adc_dev_s *self); //returns pointer to sampled data
    uint8_t data_avail; //flag showing availability of data (conversion done)
    //TODO: consider using a vtable
};

int8_t int_adc_arm(struct int_adc_dev_s *self);
uint32_t* int_adc_get_data(struct int_adc_dev_s *self);

#endif