#ifndef __DRV_ADC_H__
#define __DRV_ADC_H__

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
    uint32_t clock_prescaler;
    uint32_t resolution;
    uint32_t data_alignment;
    uint8_t eoc_selection;
    bool continuous;
    bool scan;
    bool calibration;
}adc_params_t;

typedef struct{
    dma_info_t dma;
}adc_hardware_t;

typedef struct{
    pin_info_t pin;
    uint8_t channel;
    uint32_t sample_time;
}adc_channel_t;

typedef struct{
    uint8_t sequence;
    uint32_t trigger_source;
    uint32_t trigger_mode;
}adc_trigger_t;

jxc_handle_t drv_adc_create(char *name, uint32_t adc);
void drv_adc_destroy(jxc_handle_t handle);

int drv_adc_hw_init(jxc_handle_t handle, adc_hardware_t *hw, adc_params_t *params);
int drv_adc_enable_irq(jxc_handle_t handle, uint32_t interrupt, uint8_t pre_priority, uint8_t sub_priority);
int drv_adc_disable_irq(jxc_handle_t handle, uint32_t interrupt);
FlagStatus drv_adc_interrupt_flag_get(jxc_handle_t handle, uint32_t flag);
void drv_adc_interrupt_flag_clear(jxc_handle_t handle, uint32_t flag);
FlagStatus drv_adc_flag_get(jxc_handle_t handle, uint32_t flag);
void drv_adc_flag_clear(jxc_handle_t handle, uint32_t flag);

int drv_adc_routine_channel_config(jxc_handle_t handle, adc_channel_t *channel, uint8_t rank, uint32_t length);
int drv_adc_inserted_channel_config(jxc_handle_t handle, adc_channel_t *channel, uint8_t rank, uint32_t length);
int drv_adc_trigger_config(jxc_handle_t handle, adc_trigger_t *trigger);
int drv_adc_software_trigger(jxc_handle_t handle, uint8_t sequence);
int drv_adc_read(jxc_handle_t handle, adc_channel_t *channel, uint16_t *data, uint32_t timeout);
uint16_t drv_adc_routine_data_read(jxc_handle_t handle);
uint16_t drv_adc_inserted_data_read(jxc_handle_t handle, uint8_t inserted_channel);
int drv_adc_read_dma(jxc_handle_t handle, uint16_t *data, uint32_t size);
int drv_adc_stop_dma(jxc_handle_t handle);

void drv_adc_internal_channel_enable(uint32_t function);
void drv_adc_internal_channel_disable(uint32_t function);

#ifdef __cplusplus
}
#endif

#endif
