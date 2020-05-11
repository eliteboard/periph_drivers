/* Driver for internal ADC */

#include "internal_adc.h"
#include "stm32h7xx_hal.h"
#include "errno.h"

// the dma-buffer has to be placed in the correct memory region
#define DMA_BUFFER \
    __attribute__((section(".dma_buffer"))) __attribute__ ((aligned (4)))
DMA_BUFFER uint32_t int_adc1_dma_buffer[INT_ADC_MAX_BUFFER_LENGTH]; //do not use this from the outside

int8_t int_adc_set_nsamp(struct int_adc_dev_s *self, uint16_t nsamp);
int8_t int_adc_arm(struct int_adc_dev_s *self);
uint32_t* int_adc_get_data(struct int_adc_dev_s *self);

static struct int_adc_dev_s *int_adc_devGlob=0; //required for ISR/Callback fct. works only for one instance!


void int_adc_dev_init(struct int_adc_dev_s *self, struct tim_dev_s *tim_dev, ADC_HandleTypeDef *hadc)
{
    self->hadc = hadc;
    self->set_nsamp = &int_adc_set_nsamp;
    self->arm = &int_adc_arm;
    self->get_data = &int_adc_get_data;
    self->nsamp = INT_ADC_MAX_BUFFER_LENGTH;
}

int8_t int_adc_set_nsamp(struct int_adc_dev_s *self, uint16_t nsamp)
{
    if(nsamp <= INT_ADC_MAX_BUFFER_LENGTH)
    {
        self->nsamp = nsamp;
        errno = 0;
        return 0;
    }
    else
    {
        errno = EOVERFLOW;
        return -1;
    }
}

int8_t int_adc_arm(struct int_adc_dev_s *self)
{
    if(self->nsamp > INT_ADC_MAX_BUFFER_LENGTH)
    {        
        errno = EOVERFLOW;
        return 0;
    }
    int_adc_devGlob = self;
    int_adc_devGlob->data_avail = 0;
    // conversion clock is generated by timer, arming does not collect samples until the timer is started
    // make sure that dma-buffer is in the correct memory region    
    if(HAL_ADC_Start_DMA(self->hadc, &int_adc1_dma_buffer[0], self->nsamp) != HAL_OK)
    {
        /* Start Error */
        //Error_Handler();
        return -1;
    }
    //TODO: check for errors
    errno = 0;
    return 0;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    int_adc_devGlob->data_avail = 1;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  
}

uint32_t* int_adc_get_data(struct int_adc_dev_s *self)
{
    if(self->data_avail)
    {
        self->data_avail=0; //prepare for next snapshot
        return &int_adc1_dma_buffer[0];
    }
    else
    {
        return NULL;
    }
}