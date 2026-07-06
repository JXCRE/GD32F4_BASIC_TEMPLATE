#include "main.h"
#include "bsp.h"
#include "gd32f4xx.h"                   // Device header

int main(void)
{
    bsp_systick_init();
    
    
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_PIN_2);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    
    while(1)
    {
        if(_500ms_flag){
            _500ms_flag = false;
            gpio_bit_toggle(GPIOA, GPIO_PIN_2);
            
        }
    }
}
