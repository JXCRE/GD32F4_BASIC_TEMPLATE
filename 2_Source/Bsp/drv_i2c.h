#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

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
    uint32_t clock_speed;
    uint32_t duty_cycle;
    uint32_t own_address;
    uint32_t address_format;
    uint32_t ack;
}i2c_params_t;

typedef struct{
    pin_info_t scl;
    pin_info_t sda;
    dma_info_t tx_dma;
    dma_info_t rx_dma;
}i2c_hardware_t;

jxc_handle_t drv_i2c_create(char *name, uint32_t i2c);
void drv_i2c_destroy(jxc_handle_t handle);

int drv_i2c_hw_init(jxc_handle_t handle, i2c_hardware_t *hw, i2c_params_t *params);
int drv_i2c_enable_irq(jxc_handle_t handle, i2c_interrupt_enum interrupt, uint8_t pre_priority, uint8_t sub_priority);
int drv_i2c_disable_irq(jxc_handle_t handle, i2c_interrupt_enum interrupt);
FlagStatus drv_i2c_interrupt_flag_get(jxc_handle_t handle, i2c_interrupt_flag_enum flag);
void drv_i2c_interrupt_flag_clear(jxc_handle_t handle, i2c_interrupt_flag_enum flag);
FlagStatus drv_i2c_flag_get(jxc_handle_t handle, i2c_flag_enum flag);
void drv_i2c_flag_clear(jxc_handle_t handle, i2c_flag_enum flag);
void drv_i2c_stop(jxc_handle_t handle);

int drv_i2c_master_write(jxc_handle_t handle, uint16_t addr, uint8_t *data, uint32_t size, uint32_t timeout);
int drv_i2c_master_read(jxc_handle_t handle, uint16_t addr, uint8_t *data, uint32_t size, uint32_t timeout);
int drv_i2c_mem_write(jxc_handle_t handle, uint16_t addr, uint16_t mem_addr, uint8_t mem_addr_size, uint8_t *data, uint32_t size, uint32_t timeout);
int drv_i2c_mem_read(jxc_handle_t handle, uint16_t addr, uint16_t mem_addr, uint8_t mem_addr_size, uint8_t *data, uint32_t size, uint32_t timeout);
int drv_i2c_master_write_dma(jxc_handle_t handle, uint16_t addr, uint8_t *data, uint32_t size, uint32_t timeout);
int drv_i2c_master_read_dma(jxc_handle_t handle, uint16_t addr, uint8_t *data, uint32_t size, uint32_t timeout);
int drv_i2c_mem_write_dma(jxc_handle_t handle, uint16_t addr, uint16_t mem_addr, uint8_t mem_addr_size, uint8_t *data, uint32_t size, uint32_t timeout);
int drv_i2c_mem_read_dma(jxc_handle_t handle, uint16_t addr, uint16_t mem_addr, uint8_t mem_addr_size, uint8_t *data, uint32_t size, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif
