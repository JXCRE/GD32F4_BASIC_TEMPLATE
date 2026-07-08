#include "drv_uart.h"
#include "bsp.h"
#include <stdlib.h>
#include <string.h>

typedef struct{
    uart_hardware_t hw;
    uart_params_t params;
    uint32_t uart;
    char name[10];
    uint8_t *buffer;
    uint32_t buffer_size;
}uart_handle_t;

static void drv_uart_clock_enable(uint32_t uart)
{
    switch(uart){
        case USART0:
            rcu_periph_clock_enable(RCU_USART0);
            break;
        case USART1:
            rcu_periph_clock_enable(RCU_USART1);
            break;
        case USART2:
            rcu_periph_clock_enable(RCU_USART2);
            break;
        case UART3:
            rcu_periph_clock_enable(RCU_UART3);
            break;
        case UART4:
            rcu_periph_clock_enable(RCU_UART4);
            break;
        case USART5:
            rcu_periph_clock_enable(RCU_USART5);
            break;
        case UART6:
            rcu_periph_clock_enable(RCU_UART6);
            break;
        case UART7:
            rcu_periph_clock_enable(RCU_UART7);
            break;
        default:
            break;
    }
}

static IRQn_Type drv_uart_irq_get(uint32_t uart)
{
    switch(uart){
        case USART0:
            return USART0_IRQn;
        case USART1:
            return USART1_IRQn;
        case USART2:
            return USART2_IRQn;
        case UART3:
            return UART3_IRQn;
        case UART4:
            return UART4_IRQn;
        case USART5:
            return USART5_IRQn;
#if defined(GD32F450) || defined(GD32F470)
        case UART6:
            return UART6_IRQn;
        case UART7:
            return UART7_IRQn;
#endif
        default:
            return USART0_IRQn;
    }
}

int drv_uart_enable_irq(jxc_handle_t handle, usart_interrupt_enum interrupt, uint8_t pre_priority, uint8_t sub_priority)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl)
        return -1;

    usart_interrupt_enable(hndl->uart, interrupt);
    nvic_irq_enable(drv_uart_irq_get(hndl->uart), pre_priority, sub_priority);

    return 0;
}

int drv_uart_disable_irq(jxc_handle_t handle, usart_interrupt_enum interrupt)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl)
        return -1;

    usart_interrupt_disable(hndl->uart, interrupt);
    nvic_irq_disable(drv_uart_irq_get(hndl->uart));

    return 0;
}

jxc_handle_t drv_uart_create(char *name, uint32_t uart, uint32_t buffer_size)
{
    uart_handle_t *hndl;

    if(buffer_size == 0)
        return NULL;

    hndl = (uart_handle_t *)malloc(sizeof(uart_handle_t));
    if(!hndl)
        return NULL;

    memset(hndl, 0, sizeof(uart_handle_t));
    if(name){
        strncpy(hndl->name, name, sizeof(hndl->name) - 1);
    }

    hndl->uart = uart;
    hndl->buffer_size = buffer_size;
    hndl->buffer = (uint8_t *)malloc(sizeof(uint8_t) * hndl->buffer_size);
    if(!hndl->buffer){
        free(hndl);
        return NULL;
    }
    memset(hndl->buffer, 0, hndl->buffer_size);

    return hndl;
}

void drv_uart_destroy(jxc_handle_t handle)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl)
        return;

    if(hndl->buffer){
        free(hndl->buffer);
    }
    free(hndl);
}

int drv_uart_hw_init(jxc_handle_t handle, uart_hardware_t *hw, uart_params_t *params)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl || !hw || !params)
        return -1;

    hndl->hw = *hw;
    hndl->params = *params;

    drv_uart_clock_enable(hndl->uart);

    drv_gpio_init(&hndl->hw.tx);
    drv_gpio_init(&hndl->hw.rx);

    usart_deinit(hndl->uart);
    usart_baudrate_set(hndl->uart, hndl->params.baudrate);
    usart_parity_config(hndl->uart, hndl->params.parity);
    usart_word_length_set(hndl->uart, hndl->params.databits);
    usart_stop_bit_set(hndl->uart, hndl->params.stopbits);
    usart_transmit_config(hndl->uart, USART_TRANSMIT_ENABLE);
    usart_receive_config(hndl->uart, USART_RECEIVE_ENABLE);
    usart_enable(hndl->uart);

    if(hndl->hw.tx_dma.valid){
        usart_dma_transmit_config(hndl->uart, USART_TRANSMIT_DMA_ENABLE);
        drv_dma_tx_init(&hndl->hw.tx_dma, (uint32_t)&USART_DATA(hndl->uart));
    }

    if(hndl->hw.rx_dma.valid){
        usart_dma_receive_config(hndl->uart, USART_RECEIVE_DMA_ENABLE);
        drv_dma_rx_init(&hndl->hw.rx_dma, (uint32_t)(uintptr_t)&USART_DATA(hndl->uart));
    }

    return 0;
}

int drv_uart_set_timeout(jxc_handle_t handle, uint32_t timeout)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl)
        return -1;

    usart_receiver_timeout_threshold_config(hndl->uart, timeout);
    usart_receiver_timeout_enable(hndl->uart);

    return 0;
}

FlagStatus drv_uart_interrupt_flag_get(jxc_handle_t handle, usart_interrupt_flag_enum flag)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl)
        return RESET;

    return usart_interrupt_flag_get(hndl->uart, flag);
}

void drv_uart_interrupt_flag_clear(jxc_handle_t handle, usart_interrupt_flag_enum flag)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl)
        return;

    usart_interrupt_flag_clear(hndl->uart, flag);
}

int drv_uart_write(jxc_handle_t handle, uint8_t *data, uint32_t size)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl || !data || size == 0)
        return -1;

    for(uint32_t i = 0; i < size; i++){
        usart_data_transmit(hndl->uart, data[i]);
        while(RESET == usart_flag_get(hndl->uart, USART_FLAG_TBE)){}
    }
    while(RESET == usart_flag_get(hndl->uart, USART_FLAG_TC)){}

    return 0;
}

int drv_uart_read(jxc_handle_t handle, uint8_t *data, uint32_t size, uint32_t timeout)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;
    uint32_t start;
    int read_size = 0;

    if(!hndl || !data || size == 0)
        return -1;

    start = bsp_get_sys_msTime();
    for(uint32_t i = 0; i < size; i++){
        while(RESET == usart_flag_get(hndl->uart, USART_FLAG_RBNE)){
            if((bsp_get_sys_msTime() - start) > timeout)
                return read_size;
        }
        data[i] = (uint8_t)usart_data_receive(hndl->uart);
        read_size += 1;
    }

    return read_size;
}

int drv_uart_write_dma(jxc_handle_t handle, uint8_t *data, uint32_t size)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl || !data || size == 0)
        return -1;

    return drv_dma_update_start(&hndl->hw.tx_dma, data, size);
}

int drv_uart_read_dma(jxc_handle_t handle)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl)
        return -1;

    return drv_dma_update_start(&hndl->hw.rx_dma, hndl->buffer, hndl->buffer_size);
}

uint8_t *drv_uart_buffer_get(jxc_handle_t handle)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl)
        return NULL;

    return hndl->buffer;
}

uint32_t drv_uart_buffer_size_get(jxc_handle_t handle)
{
    uart_handle_t *hndl = (uart_handle_t *)handle;

    if(!hndl)
        return 0;

    return hndl->buffer_size;
}
