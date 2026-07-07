#include "drv_gpio.h"


static void drv_gpio_clock_enable(uint32_t port)
{
    switch(port){
        case GPIOA:
            rcu_periph_clock_enable(RCU_GPIOA);
            break;
        case GPIOB:
            rcu_periph_clock_enable(RCU_GPIOB);
            break;
        case GPIOC:
            rcu_periph_clock_enable(RCU_GPIOC);
            break;
        case GPIOD:
            rcu_periph_clock_enable(RCU_GPIOD);
            break;
        case GPIOE:
            rcu_periph_clock_enable(RCU_GPIOE);
            break;
        case GPIOF:
            rcu_periph_clock_enable(RCU_GPIOF);
            break;
        case GPIOG:
            rcu_periph_clock_enable(RCU_GPIOG);
            break;
        case GPIOH:
            rcu_periph_clock_enable(RCU_GPIOH);
            break;
        case GPIOI:
            rcu_periph_clock_enable(RCU_GPIOI);
            break;
        default:
            break;
    }
}

void drv_gpio_init(pin_info_t *pin_info)
{
    if(!pin_info)
        return;

    drv_gpio_clock_enable(pin_info->port);

    gpio_mode_set(pin_info->port, pin_info->mode, pin_info->pupd, pin_info->pin);

    //GPIO function
    if(pin_info->mode == GPIO_MODE_AF)
        gpio_af_set(pin_info->port, pin_info->func, pin_info->pin);
    
    //GPIO output
    if((pin_info->mode == GPIO_MODE_OUTPUT) || (pin_info->mode == GPIO_MODE_AF)){
        gpio_output_options_set(pin_info->port, pin_info->otype, pin_info->speed, pin_info->pin);
    }
        
    if(pin_info->mode == GPIO_MODE_OUTPUT){
        if(pin_info->val)
            gpio_bit_set(pin_info->port, pin_info->pin);
        else
            gpio_bit_reset(pin_info->port, pin_info->pin);
    }
}

void drv_gpio_set(pin_info_t *pin_info, uint32_t val)
{
    if(!pin_info)
        return;

    if(val)
        gpio_bit_set(pin_info->port, pin_info->pin);
    else
        gpio_bit_reset(pin_info->port, pin_info->pin);
}

void drv_gpio_get(pin_info_t *pin_info, uint32_t *val)
{
    if(!pin_info || !val)
        return;

    if(pin_info->mode == GPIO_MODE_INPUT)
        *val = gpio_input_bit_get(pin_info->port, pin_info->pin);
}

void drv_gpio_toggle(pin_info_t *pin_info)
{
    if(!pin_info)
        return;

    gpio_bit_toggle(pin_info->port, pin_info->pin);
}



