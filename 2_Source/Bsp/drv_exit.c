#include "drv_exit.h"


static uint8_t drv_exit_port_source_get(uint32_t port)
{
    switch(port){
        case GPIOA:
            return EXTI_SOURCE_GPIOA;
        case GPIOB:
            return EXTI_SOURCE_GPIOB;
        case GPIOC:
            return EXTI_SOURCE_GPIOC;
        case GPIOD:
            return EXTI_SOURCE_GPIOD;
        case GPIOE:
            return EXTI_SOURCE_GPIOE;
        case GPIOF:
            return EXTI_SOURCE_GPIOF;
        case GPIOG:
            return EXTI_SOURCE_GPIOG;
        case GPIOH:
            return EXTI_SOURCE_GPIOH;
        case GPIOI:
            return EXTI_SOURCE_GPIOI;
        default:
            return EXTI_SOURCE_GPIOA;
    }
}

static uint8_t drv_exit_pin_source_get(uint32_t pin)
{
    uint8_t i;

    for(i = 0; i < 16; i++){
        if(pin == BIT(i)){
            return i;
        }
    }

    return EXTI_SOURCE_PIN0;
}

static IRQn_Type drv_exit_irq_get(exti_line_enum line)
{
    switch(line){
        case EXTI_0:
            return EXTI0_IRQn;
        case EXTI_1:
            return EXTI1_IRQn;
        case EXTI_2:
            return EXTI2_IRQn;
        case EXTI_3:
            return EXTI3_IRQn;
        case EXTI_4:
            return EXTI4_IRQn;
        case EXTI_5:
        case EXTI_6:
        case EXTI_7:
        case EXTI_8:
        case EXTI_9:
            return EXTI5_9_IRQn;
        default:
            return EXTI10_15_IRQn;
    }
}

void drv_exit_init(exit_info_t *exit_info)
{
    if(!exit_info)
        return;

    rcu_periph_clock_enable(RCU_SYSCFG);

    drv_gpio_init(&exit_info->gpio);
    syscfg_exti_line_config(drv_exit_port_source_get(exit_info->gpio.port), drv_exit_pin_source_get(exit_info->gpio.pin));
    //因为 gpio的pin 和 exti_line_enum 的定义是一样的，所以可以这样进行转换
    exti_init((exti_line_enum)exit_info->gpio.pin, exit_info->mode, exit_info->trig);
    exti_interrupt_flag_clear((exti_line_enum)exit_info->gpio.pin);

    if(exit_info->mode == EXTI_INTERRUPT){
        nvic_irq_enable(drv_exit_irq_get((exti_line_enum)exit_info->gpio.pin), exit_info->pre_priority, exit_info->sub_priority);
    }
}

void drv_exit_enable(exit_info_t *exit_info)
{
    if(!exit_info)
        return;

    if(exit_info->mode == EXTI_INTERRUPT){
        exti_interrupt_enable((exti_line_enum)exit_info->gpio.pin);
        nvic_irq_enable(drv_exit_irq_get((exti_line_enum)exit_info->gpio.pin), exit_info->pre_priority, exit_info->sub_priority);
    }else{
        exti_event_enable((exti_line_enum)exit_info->gpio.pin);
    }
}

void drv_exit_disable(exit_info_t *exit_info)
{
    if(!exit_info)
        return;

    if(exit_info->mode == EXTI_INTERRUPT){
        exti_interrupt_disable((exti_line_enum)exit_info->gpio.pin);
    }else{
        exti_event_disable((exti_line_enum)exit_info->gpio.pin);
    }
}

FlagStatus drv_exit_flag_get(exit_info_t *exit_info)
{
    if(!exit_info)
        return RESET;

    return exti_interrupt_flag_get((exti_line_enum)exit_info->gpio.pin);
}

void drv_exit_flag_clear(exit_info_t *exit_info)
{
    if(!exit_info)
        return;

    exti_interrupt_flag_clear((exti_line_enum)exit_info->gpio.pin);
}
