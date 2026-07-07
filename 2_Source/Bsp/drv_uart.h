#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include <stdint.h>
#include "gd32f4xx.h"
#include "drv_gpio.h"
#include "drv_dma.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef jxc_handle_t
typedef void* jxc_handle_t;
#endif

typedef struct{
    uint32_t baudrate;
    uint32_t databits;
    uint32_t stopbits;
    uint32_t parity;
}uart_params_t;

typedef struct{
    pin_info_t tx;
    pin_info_t rx;
    dma_info_t tx_dma;
    dma_info_t rx_dma;
}uart_hardware_t;

jxc_handle_t drv_uart_create(char *name, uint32_t uart, uint32_t buffer_size);
void drv_uart_destroy(jxc_handle_t handle);
int drv_uart_hw_init(jxc_handle_t handle, uart_hardware_t *hw, uart_params_t *params);
int drv_uart_set_timeout(jxc_handle_t handle, uint32_t timeout);
int drv_uart_enable_irq(jxc_handle_t handle, usart_interrupt_enum interrupt);
int drv_uart_disable_irq(jxc_handle_t handle, usart_interrupt_enum interrupt);
int drv_uart_write(jxc_handle_t handle, uint8_t *data, uint32_t size);
int drv_uart_read(jxc_handle_t handle, uint8_t *data, uint32_t size, uint32_t timeout);
int drv_uart_write_dma(jxc_handle_t handle, uint8_t *data, uint32_t size);
int drv_uart_read_dma_by_extern(jxc_handle_t handle, uint8_t *data, uint32_t size);
int drv_uart_read_dma(jxc_handle_t handle);
uint8_t *drv_uart_buffer_get(jxc_handle_t handle);
uint32_t drv_uart_buffer_size_get(jxc_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
