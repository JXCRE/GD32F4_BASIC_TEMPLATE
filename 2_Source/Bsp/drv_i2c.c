#include "drv_i2c.h"
#include "bsp.h"
#include <stdlib.h>
#include <string.h>

typedef struct{
    i2c_hardware_t hw;
    i2c_params_t params;
    uint32_t i2c;
    char name[10];
}i2c_handle_t;

static void drv_i2c_clock_enable(uint32_t i2c)
{
    switch(i2c){
        case I2C0:
            rcu_periph_clock_enable(RCU_I2C0);
            break;
        case I2C1:
            rcu_periph_clock_enable(RCU_I2C1);
            break;
        case I2C2:
            rcu_periph_clock_enable(RCU_I2C2);
            break;
        default:
            break;
    }
}

static IRQn_Type drv_i2c_ev_irq_get(uint32_t i2c)
{
    switch(i2c){
        case I2C0:
            return I2C0_EV_IRQn;
        case I2C1:
            return I2C1_EV_IRQn;
        case I2C2:
            return I2C2_EV_IRQn;
        default:
            return I2C0_EV_IRQn;
    }
}

static IRQn_Type drv_i2c_er_irq_get(uint32_t i2c)
{
    switch(i2c){
        case I2C0:
            return I2C0_ER_IRQn;
        case I2C1:
            return I2C1_ER_IRQn;
        case I2C2:
            return I2C2_ER_IRQn;
        default:
            return I2C0_ER_IRQn;
    }
}

static int drv_i2c_wait_flag(uint32_t i2c, i2c_flag_enum flag, FlagStatus status, uint32_t timeout)
{
    uint32_t start = bsp_get_sys_msTime();

    while(status != i2c_flag_get(i2c, flag)){
        if((bsp_get_sys_msTime() - start) > timeout)
            return -1;
    }

    return 0;
}

static uint32_t drv_i2c_addr_7bit(uint8_t addr)
{
    return ((uint32_t)addr) << 1;
}

static uint8_t drv_i2c_addr10_header(uint16_t addr, uint32_t direction)
{
    uint8_t header = (uint8_t)(0xF0U | (((uint32_t)addr >> 7) & 0x06U));

    if(direction == I2C_RECEIVER){
        header |= 0x01U;
    }

    return header;
}

static int drv_i2c_start_addr7(uint32_t i2c, uint16_t addr, uint32_t direction, uint32_t timeout, uint8_t wait_bus_idle)
{
    if(wait_bus_idle && drv_i2c_wait_flag(i2c, I2C_FLAG_I2CBSY, RESET, timeout) != 0)
        return -1;

    i2c_start_on_bus(i2c);
    if(drv_i2c_wait_flag(i2c, I2C_FLAG_SBSEND, SET, timeout) != 0)
        return -1;

    i2c_master_addressing(i2c, drv_i2c_addr_7bit((uint8_t)addr), direction);
    if(drv_i2c_wait_flag(i2c, I2C_FLAG_ADDSEND, SET, timeout) != 0)
        return -1;

    return 0;
}

static int drv_i2c_start_addr10_write(uint32_t i2c, uint16_t addr, uint32_t timeout, uint8_t wait_bus_idle)
{
    if(wait_bus_idle && drv_i2c_wait_flag(i2c, I2C_FLAG_I2CBSY, RESET, timeout) != 0)
        return -1;

    i2c_start_on_bus(i2c);
    if(drv_i2c_wait_flag(i2c, I2C_FLAG_SBSEND, SET, timeout) != 0)
        return -1;

    i2c_data_transmit(i2c, drv_i2c_addr10_header(addr, I2C_TRANSMITTER));
    if(drv_i2c_wait_flag(i2c, I2C_FLAG_ADD10SEND, SET, timeout) != 0)
        return -1;

    i2c_data_transmit(i2c, (uint8_t)addr);
    if(drv_i2c_wait_flag(i2c, I2C_FLAG_ADDSEND, SET, timeout) != 0)
        return -1;

    return 0;
}

static int drv_i2c_start_addr10_read(uint32_t i2c, uint16_t addr, uint32_t timeout, uint8_t wait_bus_idle)
{
    if(drv_i2c_start_addr10_write(i2c, addr, timeout, wait_bus_idle) != 0)
        return -1;

    i2c_flag_clear(i2c, I2C_FLAG_ADDSEND);
    i2c_start_on_bus(i2c);
    if(drv_i2c_wait_flag(i2c, I2C_FLAG_SBSEND, SET, timeout) != 0)
        return -1;

    i2c_data_transmit(i2c, drv_i2c_addr10_header(addr, I2C_RECEIVER));
    if(drv_i2c_wait_flag(i2c, I2C_FLAG_ADDSEND, SET, timeout) != 0)
        return -1;

    return 0;
}

static int drv_i2c_start_addr10_read_after_write(uint32_t i2c, uint16_t addr, uint32_t timeout)
{
    i2c_start_on_bus(i2c);
    if(drv_i2c_wait_flag(i2c, I2C_FLAG_SBSEND, SET, timeout) != 0)
        return -1;

    i2c_data_transmit(i2c, drv_i2c_addr10_header(addr, I2C_RECEIVER));
    if(drv_i2c_wait_flag(i2c, I2C_FLAG_ADDSEND, SET, timeout) != 0)
        return -1;

    return 0;
}

static int drv_i2c_start_addr(i2c_handle_t *hndl, uint16_t addr, uint32_t direction, uint32_t timeout, uint8_t wait_bus_idle)
{
    if(hndl->params.address_format == I2C_ADDFORMAT_10BITS){
        if(direction == I2C_RECEIVER)
            return drv_i2c_start_addr10_read(hndl->i2c, addr, timeout, wait_bus_idle);
        else
            return drv_i2c_start_addr10_write(hndl->i2c, addr, timeout, wait_bus_idle);
    }

    return drv_i2c_start_addr7(hndl->i2c, addr, direction, timeout, wait_bus_idle);
}

static int drv_i2c_start_read_after_write(i2c_handle_t *hndl, uint16_t addr, uint32_t timeout)
{
    if(hndl->params.address_format == I2C_ADDFORMAT_10BITS){
        return drv_i2c_start_addr10_read_after_write(hndl->i2c, addr, timeout);
    }

    return drv_i2c_start_addr7(hndl->i2c, addr, I2C_RECEIVER, timeout, 0U);
}

static int drv_i2c_send_mem_addr(uint32_t i2c, uint16_t mem_addr, uint8_t mem_addr_size, uint32_t timeout)
{
    if(mem_addr_size == 2U){
        if(drv_i2c_wait_flag(i2c, I2C_FLAG_TBE, SET, timeout) != 0)
            return -1;
        i2c_data_transmit(i2c, (uint8_t)(mem_addr >> 8));
    }

    if(drv_i2c_wait_flag(i2c, I2C_FLAG_TBE, SET, timeout) != 0)
        return -1;
    i2c_data_transmit(i2c, (uint8_t)mem_addr);

    return 0;
}

jxc_handle_t drv_i2c_create(char *name, uint32_t i2c)
{
    i2c_handle_t *hndl;

    hndl = (i2c_handle_t *)malloc(sizeof(i2c_handle_t));
    if(!hndl)
        return NULL;

    memset(hndl, 0, sizeof(i2c_handle_t));
    if(name){
        strncpy(hndl->name, name, sizeof(hndl->name) - 1);
    }

    hndl->i2c = i2c;

    return hndl;
}

void drv_i2c_destroy(jxc_handle_t handle)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl)
        return;

    free(hndl);
}

int drv_i2c_hw_init(jxc_handle_t handle, i2c_hardware_t *hw, i2c_params_t *params)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl || !hw || !params)
        return -1;

    hndl->hw = *hw;
    hndl->params = *params;

    drv_i2c_clock_enable(hndl->i2c);

    drv_gpio_init(&hndl->hw.scl);
    drv_gpio_init(&hndl->hw.sda);

    i2c_deinit(hndl->i2c);
    i2c_clock_config(hndl->i2c, hndl->params.clock_speed, hndl->params.duty_cycle);
    i2c_mode_addr_config(hndl->i2c, I2C_I2CMODE_ENABLE, hndl->params.address_format, hndl->params.own_address);
    i2c_ack_config(hndl->i2c, hndl->params.ack);
    i2c_enable(hndl->i2c);

    if(hndl->hw.tx_dma.valid){
        i2c_dma_config(hndl->i2c, I2C_DMA_ON);
        drv_dma_tx_init(&hndl->hw.tx_dma, (uint32_t)&I2C_DATA(hndl->i2c));
    }

    if(hndl->hw.rx_dma.valid){
        i2c_dma_config(hndl->i2c, I2C_DMA_ON);
        drv_dma_rx_init(&hndl->hw.rx_dma, (uint32_t)&I2C_DATA(hndl->i2c));
    }

    return 0;
}

int drv_i2c_enable_irq(jxc_handle_t handle, i2c_interrupt_enum interrupt, uint8_t pre_priority, uint8_t sub_priority)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl)
        return -1;

    i2c_interrupt_enable(hndl->i2c, interrupt);
    if(interrupt == I2C_INT_ERR){
        nvic_irq_enable(drv_i2c_er_irq_get(hndl->i2c), pre_priority, sub_priority);
    }else{
        nvic_irq_enable(drv_i2c_ev_irq_get(hndl->i2c), pre_priority, sub_priority);
    }

    return 0;
}

int drv_i2c_disable_irq(jxc_handle_t handle, i2c_interrupt_enum interrupt)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl)
        return -1;

    i2c_interrupt_disable(hndl->i2c, interrupt);
    if(interrupt == I2C_INT_ERR){
        nvic_irq_disable(drv_i2c_er_irq_get(hndl->i2c));
    }else{
        nvic_irq_disable(drv_i2c_ev_irq_get(hndl->i2c));
    }

    return 0;
}

FlagStatus drv_i2c_interrupt_flag_get(jxc_handle_t handle, i2c_interrupt_flag_enum flag)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl)
        return RESET;

    return i2c_interrupt_flag_get(hndl->i2c, flag);
}

void drv_i2c_interrupt_flag_clear(jxc_handle_t handle, i2c_interrupt_flag_enum flag)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl)
        return;

    i2c_interrupt_flag_clear(hndl->i2c, flag);
}

FlagStatus drv_i2c_flag_get(jxc_handle_t handle, i2c_flag_enum flag)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl)
        return RESET;

    return i2c_flag_get(hndl->i2c, flag);
}

void drv_i2c_flag_clear(jxc_handle_t handle, i2c_flag_enum flag)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl)
        return;

    i2c_flag_clear(hndl->i2c, flag);
}

void drv_i2c_stop(jxc_handle_t handle)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl)
        return;

    i2c_stop_on_bus(hndl->i2c);
}

int drv_i2c_master_write(jxc_handle_t handle, uint16_t addr, uint8_t *data, uint32_t size, uint32_t timeout)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl || !data || size == 0)
        return -1;

    if(drv_i2c_start_addr(hndl, addr, I2C_TRANSMITTER, timeout, 1U) != 0)
        return -2;
    i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);

    for(uint32_t i = 0; i < size; i++){
        if(drv_i2c_wait_flag(hndl->i2c, I2C_FLAG_TBE, SET, timeout) != 0)
            return -2;
        i2c_data_transmit(hndl->i2c, data[i]);
    }

    if(drv_i2c_wait_flag(hndl->i2c, I2C_FLAG_BTC, SET, timeout) != 0)
        return -2;

    i2c_stop_on_bus(hndl->i2c);

    return 0;
}

int drv_i2c_master_read(jxc_handle_t handle, uint16_t addr, uint8_t *data, uint32_t size, uint32_t timeout)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl || !data || size == 0)
        return -1;

    if(drv_i2c_start_addr(hndl, addr, I2C_RECEIVER, timeout, 1U) != 0)
        return -2;

    if(size == 1U){
        i2c_ack_config(hndl->i2c, I2C_ACK_DISABLE);
        i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);
        i2c_stop_on_bus(hndl->i2c);
    }else{
        i2c_ack_config(hndl->i2c, I2C_ACK_ENABLE);
        i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);
    }

    for(uint32_t i = 0; i < size; i++){
        if(i == (size - 1U)){
            i2c_ack_config(hndl->i2c, I2C_ACK_DISABLE);
            i2c_stop_on_bus(hndl->i2c);
        }

        if(drv_i2c_wait_flag(hndl->i2c, I2C_FLAG_RBNE, SET, timeout) != 0)
            return -2;
        data[i] = i2c_data_receive(hndl->i2c);
    }

    i2c_ack_config(hndl->i2c, hndl->params.ack);

    return 0;
}

int drv_i2c_mem_write(jxc_handle_t handle, uint16_t addr, uint16_t mem_addr, uint8_t mem_addr_size, uint8_t *data, uint32_t size, uint32_t timeout)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl || !data || size == 0 || (mem_addr_size != 1U && mem_addr_size != 2U))
        return -1;

    if(drv_i2c_start_addr(hndl, addr, I2C_TRANSMITTER, timeout, 1U) != 0)
        return -2;
    i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);

    if(drv_i2c_send_mem_addr(hndl->i2c, mem_addr, mem_addr_size, timeout) != 0)
        return -2;

    for(uint32_t i = 0; i < size; i++){
        if(drv_i2c_wait_flag(hndl->i2c, I2C_FLAG_TBE, SET, timeout) != 0)
            return -2;
        i2c_data_transmit(hndl->i2c, data[i]);
    }

    if(drv_i2c_wait_flag(hndl->i2c, I2C_FLAG_BTC, SET, timeout) != 0)
        return -2;

    i2c_stop_on_bus(hndl->i2c);

    return 0;
}

int drv_i2c_mem_read(jxc_handle_t handle, uint16_t addr, uint16_t mem_addr, uint8_t mem_addr_size, uint8_t *data, uint32_t size, uint32_t timeout)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl || !data || size == 0 || (mem_addr_size != 1U && mem_addr_size != 2U))
        return -1;

    if(drv_i2c_start_addr(hndl, addr, I2C_TRANSMITTER, timeout, 1U) != 0)
        return -2;
    i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);

    if(drv_i2c_send_mem_addr(hndl->i2c, mem_addr, mem_addr_size, timeout) != 0)
        return -2;

    if(drv_i2c_wait_flag(hndl->i2c, I2C_FLAG_BTC, SET, timeout) != 0)
        return -2;

    if(drv_i2c_start_read_after_write(hndl, addr, timeout) != 0)
        return -2;

    if(size == 1U){
        i2c_ack_config(hndl->i2c, I2C_ACK_DISABLE);
        i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);
        i2c_stop_on_bus(hndl->i2c);
    }else{
        i2c_ack_config(hndl->i2c, I2C_ACK_ENABLE);
        i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);
    }

    for(uint32_t i = 0; i < size; i++){
        if(i == (size - 1U)){
            i2c_ack_config(hndl->i2c, I2C_ACK_DISABLE);
            i2c_stop_on_bus(hndl->i2c);
        }

        if(drv_i2c_wait_flag(hndl->i2c, I2C_FLAG_RBNE, SET, timeout) != 0)
            return -2;
        data[i] = i2c_data_receive(hndl->i2c);
    }

    i2c_ack_config(hndl->i2c, hndl->params.ack);

    return 0;
}

int drv_i2c_master_write_dma(jxc_handle_t handle, uint16_t addr, uint8_t *data, uint32_t size, uint32_t timeout)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl || !data || size == 0)
        return -1;

    if(drv_i2c_start_addr(hndl, addr, I2C_TRANSMITTER, timeout, 1U) != 0)
        return -2;
    i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);

    return drv_dma_update_start(&hndl->hw.tx_dma, data, size);
}

int drv_i2c_master_read_dma(jxc_handle_t handle, uint16_t addr, uint8_t *data, uint32_t size, uint32_t timeout)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl || !data || size == 0)
        return -1;

    if(drv_i2c_start_addr(hndl, addr, I2C_RECEIVER, timeout, 1U) != 0)
        return -2;

    i2c_ack_config(hndl->i2c, I2C_ACK_ENABLE);
    i2c_dma_last_transfer_config(hndl->i2c, I2C_DMALST_ON);
    if(drv_dma_update_start(&hndl->hw.rx_dma, data, size) != 0)
        return -1;
    i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);

    return 0;
}

int drv_i2c_mem_write_dma(jxc_handle_t handle, uint16_t addr, uint16_t mem_addr, uint8_t mem_addr_size, uint8_t *data, uint32_t size, uint32_t timeout)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl || !data || size == 0 || (mem_addr_size != 1U && mem_addr_size != 2U))
        return -1;

    if(drv_i2c_start_addr(hndl, addr, I2C_TRANSMITTER, timeout, 1U) != 0)
        return -2;
    i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);

    if(drv_i2c_send_mem_addr(hndl->i2c, mem_addr, mem_addr_size, timeout) != 0)
        return -2;

    return drv_dma_update_start(&hndl->hw.tx_dma, data, size);
}

int drv_i2c_mem_read_dma(jxc_handle_t handle, uint16_t addr, uint16_t mem_addr, uint8_t mem_addr_size, uint8_t *data, uint32_t size, uint32_t timeout)
{
    i2c_handle_t *hndl = (i2c_handle_t *)handle;

    if(!hndl || !data || size == 0 || (mem_addr_size != 1U && mem_addr_size != 2U))
        return -1;

    if(drv_i2c_start_addr(hndl, addr, I2C_TRANSMITTER, timeout, 1U) != 0)
        return -2;
    i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);

    if(drv_i2c_send_mem_addr(hndl->i2c, mem_addr, mem_addr_size, timeout) != 0)
        return -2;

    if(drv_i2c_wait_flag(hndl->i2c, I2C_FLAG_BTC, SET, timeout) != 0)
        return -2;

    if(drv_i2c_start_read_after_write(hndl, addr, timeout) != 0)
        return -2;

    i2c_ack_config(hndl->i2c, I2C_ACK_ENABLE);
    i2c_dma_last_transfer_config(hndl->i2c, I2C_DMALST_ON);
    if(drv_dma_update_start(&hndl->hw.rx_dma, data, size) != 0)
        return -1;
    i2c_flag_clear(hndl->i2c, I2C_FLAG_ADDSEND);

    return 0;
}
