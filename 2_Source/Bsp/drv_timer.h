#ifndef __DRV_TIMER_H__
#define __DRV_TIMER_H__

#include <stdint.h>
#include <stdbool.h>
#include "gd32f4xx.h"
#include "drv_gpio.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef jxc_handle_t
typedef void* jxc_handle_t;
#endif

typedef timer_parameter_struct          timer_base_params_t;
typedef timer_oc_parameter_struct       timer_oc_params_t;
typedef timer_ic_parameter_struct       timer_ic_params_t;
typedef timer_break_parameter_struct    timer_break_params_t;

typedef struct{
    pin_info_t pin;
    uint16_t channel;
    uint16_t mode;
    uint32_t pulse;
    timer_oc_params_t params;
}timer_pwm_channel_t;

typedef struct{
    pin_info_t pin;
    uint16_t channel;
    timer_ic_params_t params;
}timer_ic_channel_t;

typedef struct{
    pin_info_t ch0;
    pin_info_t ch1;
    uint32_t mode;
    uint16_t ch0_polarity;
    uint16_t ch1_polarity;
}timer_encoder_params_t;

jxc_handle_t drv_timer_create(char *name, uint32_t timer);
void drv_timer_destroy(jxc_handle_t handle);

int drv_timer_base_init(jxc_handle_t handle, timer_base_params_t *params);
int drv_timer_start(jxc_handle_t handle);
int drv_timer_stop(jxc_handle_t handle);
int drv_timer_counter_set(jxc_handle_t handle, uint32_t counter);
uint32_t drv_timer_counter_get(jxc_handle_t handle);
int drv_timer_autoreload_set(jxc_handle_t handle, uint32_t autoreload);
int drv_timer_prescaler_set(jxc_handle_t handle, uint16_t prescaler, uint8_t reload);

int drv_timer_enable_irq(jxc_handle_t handle, uint32_t interrupt, uint8_t pre_priority, uint8_t sub_priority);
int drv_timer_disable_irq(jxc_handle_t handle, uint32_t interrupt);
FlagStatus drv_timer_interrupt_flag_get(jxc_handle_t handle, uint32_t interrupt);
void drv_timer_interrupt_flag_clear(jxc_handle_t handle, uint32_t interrupt);
FlagStatus drv_timer_flag_get(jxc_handle_t handle, uint32_t flag);
void drv_timer_flag_clear(jxc_handle_t handle, uint32_t flag);

int drv_timer_pwm_channel_init(jxc_handle_t handle, timer_pwm_channel_t *channel);
int drv_timer_pwm_set_pulse(jxc_handle_t handle, uint16_t channel, uint32_t pulse);
int drv_timer_pwm_channel_enable(jxc_handle_t handle, uint16_t channel);
int drv_timer_pwm_channel_disable(jxc_handle_t handle, uint16_t channel);

int drv_timer_ic_channel_init(jxc_handle_t handle, timer_ic_channel_t *channel);
uint32_t drv_timer_capture_get(jxc_handle_t handle, uint16_t channel);

int drv_timer_encoder_init(jxc_handle_t handle, timer_encoder_params_t *params);

int drv_timer_dma_enable(jxc_handle_t handle, uint16_t dma);
int drv_timer_dma_disable(jxc_handle_t handle, uint16_t dma);
int drv_timer_dma_request_source_select(jxc_handle_t handle, uint8_t dma_request);
int drv_timer_dma_transfer_config(jxc_handle_t handle, uint32_t dma_baseaddr, uint32_t dma_length);

int drv_timer_break_deadtime_config(jxc_handle_t handle, timer_break_params_t *params);
int drv_timer_primary_output_config(jxc_handle_t handle, ControlStatus state);

#ifdef __cplusplus
}
#endif

#endif
