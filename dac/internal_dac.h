/* Driver for internal ADC */

#ifndef INT_DAC_H
#define INT_DAC_H

#include <stdint.h>
#include "tim_hal.h"

//extern uint16_t int_dac1_dma_buffer[], int_dac2_dma_buffer[];
#define INT_DAC_BUFFER_LENGTH (4096)

struct int_dac_dev_s
{    
    DAC_HandleTypeDef *hdac;
    struct tim_dev_s *tim_dev;
    int8_t (*set_sample) (struct int_dac_dev_s *self, uint16_t val, uint16_t idx);
    int8_t (*fill_buf) (struct int_dac_dev_s *self);
    int8_t (*arm) (struct int_dac_dev_s *self);
    //TODO: consider using a vtable
};

int8_t int_dac_set_sample(struct int_dac_dev_s *self, uint16_t val, uint16_t idx);
int8_t int_dac_fill_buf(struct int_dac_dev_s *self, uint16_t *data);
int8_t int_dac_arm(struct int_dac_dev_s *self);


#endif