#ifndef __DRV_DAC_H__
#define __DRV_DAC_H__

#include <stdint.h>
#include <stdbool.h>
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
    pin_info_t out0;
    pin_info_t out1;
    bool out0_valid;
    bool out1_valid;
    dma_info_t out0_dma;
    dma_info_t out1_dma;
}dac_hardware_t;

typedef struct{
    uint8_t out;
    uint32_t trigger_source;
    uint32_t wave_mode;
    uint32_t wave_bits;
    bool trigger_enable;
    bool buffer_enable;
    bool dma_enable;
}dac_channel_t;

jxc_handle_t drv_dac_create(char *name, uint32_t dac);
void drv_dac_destroy(jxc_handle_t handle);

int drv_dac_hw_init(jxc_handle_t handle, dac_hardware_t *hw);
int drv_dac_channel_init(jxc_handle_t handle, dac_channel_t *channel);
int drv_dac_enable_irq(jxc_handle_t handle, uint32_t interrupt, uint8_t pre_priority, uint8_t sub_priority);
int drv_dac_disable_irq(jxc_handle_t handle, uint32_t interrupt);
FlagStatus drv_dac_interrupt_flag_get(jxc_handle_t handle, uint32_t flag);
void drv_dac_interrupt_flag_clear(jxc_handle_t handle, uint32_t flag);
FlagStatus drv_dac_flag_get(jxc_handle_t handle, uint32_t flag);
void drv_dac_flag_clear(jxc_handle_t handle, uint32_t flag);

int drv_dac_enable(jxc_handle_t handle, uint8_t out);
int drv_dac_disable(jxc_handle_t handle, uint8_t out);
int drv_dac_data_set(jxc_handle_t handle, uint8_t out, uint32_t align, uint16_t data);
uint16_t drv_dac_output_get(jxc_handle_t handle, uint8_t out);
int drv_dac_software_trigger(jxc_handle_t handle, uint8_t out);
int drv_dac_write_dma(jxc_handle_t handle, uint8_t out, uint32_t align, uint8_t *data, uint32_t size);
int drv_dac_stop_dma(jxc_handle_t handle, uint8_t out);

#ifdef __cplusplus
}
#endif

#endif
