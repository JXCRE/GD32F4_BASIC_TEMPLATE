#include "drv_gpio.h"


void drv_gpio_init(pin_info_t *pin_info)
{
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
    if(val)
        gpio_bit_set(pin_info->port, pin_info->pin);
    else
        gpio_bit_reset(pin_info->port, pin_info->pin);
}

void drv_gpio_get(pin_info_t *pin_info, uint32_t *val)
{
    if(pin_info->mode == GPIO_MODE_INPUT)
        *val = gpio_input_bit_get(pin_info->port, pin_info->pin);
}

void drv_gpio_toggle(pin_info_t *pin_info)
{
    gpio_bit_toggle(pin_info->port, pin_info->pin);
}



