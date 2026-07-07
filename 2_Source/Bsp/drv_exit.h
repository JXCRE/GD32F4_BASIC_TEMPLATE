#ifndef __DRV_EXIT_H__
#define __DRV_EXIT_H__

#include <stdint.h>
#include "gd32f4xx.h"
#include "drv_gpio.h"

/**
 * 实现gpio输入引脚的外部中断功能，支持中断和事件两种模式，支持上升沿、下降沿、双边沿触发
 */

typedef struct{
    pin_info_t          gpio;           //GPIO input pin
    exti_mode_enum      mode;           //EXTI_INTERRUPT/EXTI_EVENT
    exti_trig_type_enum trig;           //EXTI_TRIG_x
    uint8_t             pre_priority;   //NVIC pre-emption priority
    uint8_t             sub_priority;   //NVIC subpriority
}exit_info_t;

#define EXIT_INFO_EX(gpio_a, mode_a, trig_a, pre_priority_a, sub_priority_a) \
    ((exit_info_t){ \
        .gpio = gpio_a, \
        .mode = mode_a, \
        .trig = trig_a, \
        .pre_priority = pre_priority_a, \
        .sub_priority = sub_priority_a \
    })

#define EXIT_GPIO_INTERRUPT(port_a, pin_a, pupd_a, trig_a, pre_priority_a, sub_priority_a) \
    EXIT_INFO_EX(PIN_INPUT(port_a, pin_a, pupd_a), EXTI_INTERRUPT, trig_a, pre_priority_a, sub_priority_a)

#define EXIT_GPIO_EVENT(port_a, pin_a, pupd_a, trig_a) \
    EXIT_INFO_EX(PIN_INPUT(port_a, pin_a, pupd_a), EXTI_EVENT, trig_a, 0, 0)

void drv_exit_init(exit_info_t *exit_info);
void drv_exit_enable(exit_info_t *exit_info);
void drv_exit_disable(exit_info_t *exit_info);
FlagStatus drv_exit_flag_get(exit_info_t *exit_info);
void drv_exit_flag_clear(exit_info_t *exit_info);

#endif
