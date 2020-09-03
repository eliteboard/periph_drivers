#ifndef __UART_H_
#define __UART_H_

#include <stm32h7xx_hal.h>
#include "serial.h"
#include "rb.h"


void uart_serial_dev_init(struct serial_dev_s* s, UART_HandleTypeDef* h, struct rb_handle_s* rxbuffer, struct rb_handle_s* txbuffer);



#endif