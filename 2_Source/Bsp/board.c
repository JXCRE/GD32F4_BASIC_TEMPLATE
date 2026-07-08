#include "board.h"

//GPIO
pin_info_t led1;    //PA2
pin_info_t led2;    //PA3

//EXIT
exit_info_t key1;   //PA0
exit_info_t key2;   //PA1

//----------------------------------------------------------------------------------------------------
void EXTI0_IRQHandler()
{
    if(RESET != drv_exit_flag_get(&key1)){
        drv_exit_flag_clear(&key1);
        // key1
        drv_gpio_toggle(&led2);
    }
}

void EXTI1_IRQHandler()
{
    if(RESET != drv_exit_flag_get(&key2)){
        drv_exit_flag_clear(&key2);
        // key2
        drv_gpio_toggle(&led2);
    }
}

//----------------------------------------------------------------------------------------------------
static void gpio_init(void)
{
    led1 = PIN_OUTPUT(GPIOA, GPIO_PIN_2, GPIO_PUPD_NONE, GPIO_OSPEED_50MHZ, 1);
    led2 = PIN_OUTPUT(GPIOA, GPIO_PIN_3, GPIO_PUPD_NONE, GPIO_OSPEED_50MHZ, 1);

    drv_gpio_init(&led1);
    drv_gpio_init(&led2);
}

static void exit_init(void)
{
    key1 = EXIT_GPIO_INTERRUPT(GPIOA, GPIO_PIN_0, GPIO_PUPD_PULLDOWN, EXTI_TRIG_FALLING, 0, 0);
    key2 = EXIT_GPIO_INTERRUPT(GPIOA, GPIO_PIN_1, GPIO_PUPD_PULLDOWN, EXTI_TRIG_RISING, 0, 0);

    drv_exit_init(&key1);
    drv_exit_enable(&key1);

    drv_exit_init(&key2);
    drv_exit_enable(&key2);
}

//----------------------------------------------------------------------------------------------------
void board_init(void)
{
    bsp_systick_init();
    bsp_nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);

    gpio_init();
    exit_init();

}

//----------------------------------------------------------------------------------------------------
void app_run(void)
{
    if(_500ms_flag){
        _500ms_flag = false;
        drv_gpio_toggle(&led1);
    }
}
