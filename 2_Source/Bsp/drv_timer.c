#include "drv_timer.h"
#include <stdlib.h>
#include <string.h>

typedef struct{
    timer_base_params_t params;
    uint32_t timer;
    char name[10];
}timer_handle_t;

static void drv_timer_clock_enable(uint32_t timer)
{
    switch(timer){
        case TIMER0:
            rcu_periph_clock_enable(RCU_TIMER0);
            break;
        case TIMER1:
            rcu_periph_clock_enable(RCU_TIMER1);
            break;
        case TIMER2:
            rcu_periph_clock_enable(RCU_TIMER2);
            break;
        case TIMER3:
            rcu_periph_clock_enable(RCU_TIMER3);
            break;
        case TIMER4:
            rcu_periph_clock_enable(RCU_TIMER4);
            break;
        case TIMER5:
            rcu_periph_clock_enable(RCU_TIMER5);
            break;
        case TIMER6:
            rcu_periph_clock_enable(RCU_TIMER6);
            break;
        case TIMER7:
            rcu_periph_clock_enable(RCU_TIMER7);
            break;
        case TIMER8:
            rcu_periph_clock_enable(RCU_TIMER8);
            break;
        case TIMER9:
            rcu_periph_clock_enable(RCU_TIMER9);
            break;
        case TIMER10:
            rcu_periph_clock_enable(RCU_TIMER10);
            break;
        case TIMER11:
            rcu_periph_clock_enable(RCU_TIMER11);
            break;
        case TIMER12:
            rcu_periph_clock_enable(RCU_TIMER12);
            break;
        case TIMER13:
            rcu_periph_clock_enable(RCU_TIMER13);
            break;
        default:
            break;
    }
}

static bool drv_timer_is_advanced(uint32_t timer)
{
    return ((timer == TIMER0) || (timer == TIMER7));
}

static bool drv_timer_is_basic(uint32_t timer)
{
    return ((timer == TIMER5) || (timer == TIMER6));
}

static bool drv_timer_has_channel(uint32_t timer)
{
    return !drv_timer_is_basic(timer);
}

static IRQn_Type drv_timer_irq_get(uint32_t timer, uint32_t interrupt)
{
    switch(timer){
        case TIMER0:
            if(interrupt == TIMER_INT_BRK)
                return TIMER0_BRK_TIMER8_IRQn;
            if(interrupt == TIMER_INT_UP)
                return TIMER0_UP_TIMER9_IRQn;
            if((interrupt == TIMER_INT_TRG) || (interrupt == TIMER_INT_CMT))
                return TIMER0_TRG_CMT_TIMER10_IRQn;
            return TIMER0_Channel_IRQn;
        case TIMER1:
            return TIMER1_IRQn;
        case TIMER2:
            return TIMER2_IRQn;
        case TIMER3:
            return TIMER3_IRQn;
        case TIMER4:
            return TIMER4_IRQn;
        case TIMER5:
            return TIMER5_DAC_IRQn;
        case TIMER6:
            return TIMER6_IRQn;
        case TIMER7:
            if(interrupt == TIMER_INT_BRK)
                return TIMER7_BRK_TIMER11_IRQn;
            if(interrupt == TIMER_INT_UP)
                return TIMER7_UP_TIMER12_IRQn;
            if((interrupt == TIMER_INT_TRG) || (interrupt == TIMER_INT_CMT))
                return TIMER7_TRG_CMT_TIMER13_IRQn;
            return TIMER7_Channel_IRQn;
        case TIMER8:
            return TIMER0_BRK_TIMER8_IRQn;
        case TIMER9:
            return TIMER0_UP_TIMER9_IRQn;
        case TIMER10:
            return TIMER0_TRG_CMT_TIMER10_IRQn;
        case TIMER11:
            return TIMER7_BRK_TIMER11_IRQn;
        case TIMER12:
            return TIMER7_UP_TIMER12_IRQn;
        case TIMER13:
            return TIMER7_TRG_CMT_TIMER13_IRQn;
        default:
            return TIMER1_IRQn;
    }
}

static bool drv_timer_channel_valid(uint16_t channel)
{
    return ((channel == TIMER_CH_0) || (channel == TIMER_CH_1) || \
            (channel == TIMER_CH_2) || (channel == TIMER_CH_3));
}

jxc_handle_t drv_timer_create(char *name, uint32_t timer)
{
    timer_handle_t *hndl;

    hndl = (timer_handle_t *)malloc(sizeof(timer_handle_t));
    if(!hndl)
        return NULL;

    memset(hndl, 0, sizeof(timer_handle_t));
    if(name){
        strncpy(hndl->name, name, sizeof(hndl->name) - 1);
    }

    hndl->timer = timer;

    return hndl;
}

void drv_timer_destroy(jxc_handle_t handle)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return;

    free(hndl);
}

int drv_timer_base_init(jxc_handle_t handle, timer_base_params_t *params)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl || !params)
        return -1;

    hndl->params = *params;

    drv_timer_clock_enable(hndl->timer);
    timer_deinit(hndl->timer);
    timer_init(hndl->timer, &hndl->params);
    timer_auto_reload_shadow_enable(hndl->timer);

    return 0;
}

int drv_timer_start(jxc_handle_t handle)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_enable(hndl->timer);

    return 0;
}

int drv_timer_stop(jxc_handle_t handle)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_disable(hndl->timer);

    return 0;
}

int drv_timer_counter_set(jxc_handle_t handle, uint32_t counter)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_counter_value_config(hndl->timer, counter);

    return 0;
}

uint32_t drv_timer_counter_get(jxc_handle_t handle)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return 0;

    return timer_counter_read(hndl->timer);
}

int drv_timer_autoreload_set(jxc_handle_t handle, uint32_t autoreload)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_autoreload_value_config(hndl->timer, autoreload);

    return 0;
}

int drv_timer_prescaler_set(jxc_handle_t handle, uint16_t prescaler, uint8_t reload)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_prescaler_config(hndl->timer, prescaler, reload);

    return 0;
}

int drv_timer_enable_irq(jxc_handle_t handle, uint32_t interrupt, uint8_t pre_priority, uint8_t sub_priority)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_interrupt_enable(hndl->timer, interrupt);
    nvic_irq_enable(drv_timer_irq_get(hndl->timer, interrupt), pre_priority, sub_priority);

    return 0;
}

int drv_timer_disable_irq(jxc_handle_t handle, uint32_t interrupt)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_interrupt_disable(hndl->timer, interrupt);
    nvic_irq_disable(drv_timer_irq_get(hndl->timer, interrupt));

    return 0;
}

FlagStatus drv_timer_interrupt_flag_get(jxc_handle_t handle, uint32_t interrupt)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return RESET;

    return timer_interrupt_flag_get(hndl->timer, interrupt);
}

void drv_timer_interrupt_flag_clear(jxc_handle_t handle, uint32_t interrupt)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return;

    timer_interrupt_flag_clear(hndl->timer, interrupt);
}

FlagStatus drv_timer_flag_get(jxc_handle_t handle, uint32_t flag)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return RESET;

    return timer_flag_get(hndl->timer, flag);
}

void drv_timer_flag_clear(jxc_handle_t handle, uint32_t flag)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return;

    timer_flag_clear(hndl->timer, flag);
}

int drv_timer_pwm_channel_init(jxc_handle_t handle, timer_pwm_channel_t *channel)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl || !channel || !drv_timer_channel_valid(channel->channel))
        return -1;
    if(!drv_timer_has_channel(hndl->timer))
        return -2;

    drv_gpio_init(&channel->pin);
    timer_channel_output_config(hndl->timer, channel->channel, &channel->params);
    timer_channel_output_mode_config(hndl->timer, channel->channel, channel->mode);
    timer_channel_output_pulse_value_config(hndl->timer, channel->channel, channel->pulse);
    timer_channel_output_shadow_config(hndl->timer, channel->channel, TIMER_OC_SHADOW_ENABLE);

    if(drv_timer_is_advanced(hndl->timer)){
        timer_primary_output_config(hndl->timer, ENABLE);
    }

    return 0;
}

int drv_timer_pwm_set_pulse(jxc_handle_t handle, uint16_t channel, uint32_t pulse)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl || !drv_timer_channel_valid(channel))
        return -1;
    if(!drv_timer_has_channel(hndl->timer))
        return -2;

    timer_channel_output_pulse_value_config(hndl->timer, channel, pulse);

    return 0;
}

int drv_timer_pwm_channel_enable(jxc_handle_t handle, uint16_t channel)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl || !drv_timer_channel_valid(channel))
        return -1;
    if(!drv_timer_has_channel(hndl->timer))
        return -2;

    timer_channel_output_state_config(hndl->timer, channel, TIMER_CCX_ENABLE);

    return 0;
}

int drv_timer_pwm_channel_disable(jxc_handle_t handle, uint16_t channel)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl || !drv_timer_channel_valid(channel))
        return -1;
    if(!drv_timer_has_channel(hndl->timer))
        return -2;

    timer_channel_output_state_config(hndl->timer, channel, TIMER_CCX_DISABLE);

    return 0;
}

int drv_timer_ic_channel_init(jxc_handle_t handle, timer_ic_channel_t *channel)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl || !channel || !drv_timer_channel_valid(channel->channel))
        return -1;
    if(!drv_timer_has_channel(hndl->timer))
        return -2;

    drv_gpio_init(&channel->pin);
    timer_input_capture_config(hndl->timer, channel->channel, &channel->params);

    return 0;
}

uint32_t drv_timer_capture_get(jxc_handle_t handle, uint16_t channel)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl || !drv_timer_channel_valid(channel))
        return 0;
    if(!drv_timer_has_channel(hndl->timer))
        return 0;

    return timer_channel_capture_value_register_read(hndl->timer, channel);
}

int drv_timer_encoder_init(jxc_handle_t handle, timer_encoder_params_t *params)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl || !params)
        return -1;
    if(!drv_timer_has_channel(hndl->timer))
        return -2;

    drv_gpio_init(&params->ch0);
    drv_gpio_init(&params->ch1);
    timer_quadrature_decoder_mode_config(hndl->timer, params->mode, params->ch0_polarity, params->ch1_polarity);

    return 0;
}

int drv_timer_dma_enable(jxc_handle_t handle, uint16_t dma)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_dma_enable(hndl->timer, dma);

    return 0;
}

int drv_timer_dma_disable(jxc_handle_t handle, uint16_t dma)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_dma_disable(hndl->timer, dma);

    return 0;
}

int drv_timer_dma_request_source_select(jxc_handle_t handle, uint8_t dma_request)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_channel_dma_request_source_select(hndl->timer, dma_request);

    return 0;
}

int drv_timer_dma_transfer_config(jxc_handle_t handle, uint32_t dma_baseaddr, uint32_t dma_length)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;

    timer_dma_transfer_config(hndl->timer, dma_baseaddr, dma_length);

    return 0;
}

int drv_timer_break_deadtime_config(jxc_handle_t handle, timer_break_params_t *params)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl || !params)
        return -1;
    if(!drv_timer_is_advanced(hndl->timer))
        return -2;

    timer_break_config(hndl->timer, params);

    return 0;
}

int drv_timer_primary_output_config(jxc_handle_t handle, ControlStatus state)
{
    timer_handle_t *hndl = (timer_handle_t *)handle;

    if(!hndl)
        return -1;
    if(!drv_timer_is_advanced(hndl->timer))
        return -2;

    timer_primary_output_config(hndl->timer, state);

    return 0;
}
