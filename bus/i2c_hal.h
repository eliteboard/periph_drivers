/* Driver for I2C based on HAL functions */
#ifndef I2C_HAL_H
#define I2C_HAL_H

#include "stm32h7xx_hal.h"

struct i2c_dev_s
{    
    I2C_HandleTypeDef *hi2c;
    int8_t (*mem_read) (struct i2c_dev_s *self, uint16_t DevAddress,
                            uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
    int8_t (*mem_write) (struct i2c_dev_s *self, uint16_t DevAddress,
                            uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
    int8_t (*master_transmit) (struct i2c_dev_s *self, uint16_t DevAddress,
                                   uint8_t *pData, uint16_t Size, uint32_t Timeout);
    //TODO: consider using a vtable
};

int8_t i2c_mem_read(struct i2c_dev_s *self, uint16_t DevAddress,
                            uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
int8_t i2c_mem_write(struct i2c_dev_s *self, uint16_t DevAddress,
                            uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
int8_t i2c_master_transmit(struct i2c_dev_s *self, uint16_t DevAddress,
                            uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif