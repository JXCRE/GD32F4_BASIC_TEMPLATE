#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include <stdint.h>
#include "gd32f4xx.h"

/**
 * 实现gpio输入输出引脚的初始化功能，支持输入、输出、复用、模拟四种模式，支持上拉、下拉、推挽、开漏等多种配置
 * ----------时钟需要外部进行配置
 */

typedef struct{
    uint32_t    port;      //
    uint32_t    pin;       //
    uint32_t    mode;      //gpio输入/输出模式
    uint32_t    pupd;      //gpio上拉/下拉模式
    uint32_t    otype;     //gpio输出类型
    uint32_t    speed;     //gpio输出速度
    uint32_t    func;      //gpio复用功能
    uint32_t    val;       //gpio默认输出电平(在输出模式下生效)
}pin_info_t;

#define PIN_INFO_EX(port_a, pin_a, mode_a, pupd_a, otype_a, speed_a, func_a, val_a) \
    ((pin_info_t){ \
        .port = port_a, \
        .pin = pin_a, \
        .mode = mode_a, \
        .pupd = pupd_a, \
        .otype = otype_a, \
        .speed = speed_a, \
        .func = func_a, \
        .val = val_a    \
    })

#define PIN_INFO(port_a, pin_a, mode_a, pupd_a, otype_a, func_a, val_a) \
    PIN_INFO_EX(port_a, pin_a, mode_a, pupd_a, otype_a, GPIO_OSPEED_50MHZ, func_a, val_a)

#define PIN_INPUT(port_a, pin_a, pupd_a) \
    PIN_INFO_EX(port_a, pin_a, GPIO_MODE_INPUT, pupd_a, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_AF_0, 0)

#define PIN_OUTPUT(port_a, pin_a, otype_a, speed_a, val_a) \
    PIN_INFO_EX(port_a, pin_a, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, otype_a, speed_a, GPIO_AF_0, val_a)

#define PIN_AF(port_a, pin_a, pupd_a, otype_a, speed_a, func_a) \
    PIN_INFO_EX(port_a, pin_a, GPIO_MODE_AF, pupd_a, otype_a, speed_a, func_a, 0)

#define PIN_ANALOG(port_a, pin_a) \
    PIN_INFO_EX(port_a, pin_a, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_AF_0, 0)

void drv_gpio_init(pin_info_t *pin_info);
void drv_gpio_set(pin_info_t *pin_info, uint32_t val);
void drv_gpio_get(pin_info_t *pin_info, uint32_t *val);
void drv_gpio_toggle(pin_info_t *pin_info);


#endif
