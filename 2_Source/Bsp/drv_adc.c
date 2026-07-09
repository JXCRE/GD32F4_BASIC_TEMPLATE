#include "drv_adc.h"
#include <stdlib.h>
#include <string.h>

typedef struct{
    char name[10];
    uint32_t adc;
    adc_hardware_t hw;
    adc_params_t params;
}drv_adc_handle_t;

static rcu_periph_enum drv_adc_clock_get(uint32_t adc)
{
    if(adc == ADC0)
        return RCU_ADC0;
    if(adc == ADC1)
        return RCU_ADC1;
    if(adc == ADC2)
        return RCU_ADC2;

    return RCU_ADC0;
}

static bool drv_adc_is_valid(drv_adc_handle_t *adc_handle)
{
    return (adc_handle && (adc_handle->adc == ADC0 || adc_handle->adc == ADC1 || adc_handle->adc == ADC2));
}

static void drv_adc_params_default(adc_params_t *params)
{
    params->clock_prescaler = ADC_ADCCK_PCLK2_DIV6;
    params->resolution = ADC_RESOLUTION_12B;
    params->data_alignment = ADC_DATAALIGN_RIGHT;
    params->eoc_selection = ADC_EOC_SET_SEQUENCE;
    params->continuous = false;
    params->scan = false;
    params->calibration = true;
}

static void drv_adc_channel_gpio_init(adc_channel_t *channel)
{
    if(channel->channel <= ADC_CHANNEL_15){
        channel->pin.mode = GPIO_MODE_ANALOG;
        channel->pin.pupd = GPIO_PUPD_NONE;
        channel->pin.otype = GPIO_OTYPE_PP;
        channel->pin.func = GPIO_AF_0;
        drv_gpio_init(&channel->pin);
    }
}

jxc_handle_t drv_adc_create(char *name, uint32_t adc)
{
    drv_adc_handle_t *adc_handle;

    adc_handle = (drv_adc_handle_t *)malloc(sizeof(drv_adc_handle_t));
    if(!adc_handle)
        return NULL;

    memset(adc_handle, 0, sizeof(drv_adc_handle_t));
    if(name)
        strncpy(adc_handle->name, name, sizeof(adc_handle->name) - 1);
    adc_handle->adc = adc;
    drv_adc_params_default(&adc_handle->params);

    return (jxc_handle_t)adc_handle;
}

void drv_adc_destroy(jxc_handle_t handle)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!adc_handle)
        return;

    adc_disable(adc_handle->adc);
    free(adc_handle);
}

int drv_adc_hw_init(jxc_handle_t handle, adc_hardware_t *hw, adc_params_t *params)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return -1;

    rcu_periph_clock_enable(drv_adc_clock_get(adc_handle->adc));

    if(hw)
        adc_handle->hw = *hw;
    if(params)
        adc_handle->params = *params;

    adc_clock_config(adc_handle->params.clock_prescaler);
    adc_disable(adc_handle->adc);
    adc_resolution_config(adc_handle->adc, adc_handle->params.resolution);
    adc_data_alignment_config(adc_handle->adc, adc_handle->params.data_alignment);
    adc_end_of_conversion_config(adc_handle->adc, adc_handle->params.eoc_selection);
    adc_special_function_config(adc_handle->adc, ADC_CONTINUOUS_MODE, adc_handle->params.continuous ? ENABLE : DISABLE);
    adc_special_function_config(adc_handle->adc, ADC_SCAN_MODE, adc_handle->params.scan ? ENABLE : DISABLE);

    if(adc_handle->hw.dma.valid){
        adc_handle->hw.dma.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;
        drv_dma_rx_init(&adc_handle->hw.dma, (uint32_t)&ADC_RDATA(adc_handle->adc));
        adc_dma_mode_enable(adc_handle->adc);
        adc_dma_request_after_last_enable(adc_handle->adc);
    }else{
        adc_dma_mode_disable(adc_handle->adc);
    }

    adc_enable(adc_handle->adc);
    if(adc_handle->params.calibration)
        adc_calibration_enable(adc_handle->adc);

    return 0;
}

int drv_adc_enable_irq(jxc_handle_t handle, uint32_t interrupt, uint8_t pre_priority, uint8_t sub_priority)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return -1;

    adc_interrupt_enable(adc_handle->adc, interrupt);
    nvic_irq_enable(ADC_IRQn, pre_priority, sub_priority);

    return 0;
}

int drv_adc_disable_irq(jxc_handle_t handle, uint32_t interrupt)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return -1;

    adc_interrupt_disable(adc_handle->adc, interrupt);
    nvic_irq_disable(ADC_IRQn);

    return 0;
}

FlagStatus drv_adc_interrupt_flag_get(jxc_handle_t handle, uint32_t flag)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return RESET;

    return adc_interrupt_flag_get(adc_handle->adc, flag);
}

void drv_adc_interrupt_flag_clear(jxc_handle_t handle, uint32_t flag)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return;

    adc_interrupt_flag_clear(adc_handle->adc, flag);
}

FlagStatus drv_adc_flag_get(jxc_handle_t handle, uint32_t flag)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return RESET;

    return adc_flag_get(adc_handle->adc, flag);
}

void drv_adc_flag_clear(jxc_handle_t handle, uint32_t flag)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return;

    adc_flag_clear(adc_handle->adc, flag);
}

int drv_adc_routine_channel_config(jxc_handle_t handle, adc_channel_t *channel, uint8_t rank, uint32_t length)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle) || !channel || length == 0)
        return -1;

    drv_adc_channel_gpio_init(channel);
    adc_channel_length_config(adc_handle->adc, ADC_ROUTINE_CHANNEL, length);
    adc_routine_channel_config(adc_handle->adc, rank, channel->channel, channel->sample_time);

    return 0;
}

int drv_adc_inserted_channel_config(jxc_handle_t handle, adc_channel_t *channel, uint8_t rank, uint32_t length)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle) || !channel || length == 0)
        return -1;

    drv_adc_channel_gpio_init(channel);
    adc_channel_length_config(adc_handle->adc, ADC_INSERTED_CHANNEL, length);
    adc_inserted_channel_config(adc_handle->adc, rank, channel->channel, channel->sample_time);

    return 0;
}

int drv_adc_trigger_config(jxc_handle_t handle, adc_trigger_t *trigger)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle) || !trigger)
        return -1;

    adc_external_trigger_source_config(adc_handle->adc, trigger->sequence, trigger->trigger_source);
    adc_external_trigger_config(adc_handle->adc, trigger->sequence, trigger->trigger_mode);

    return 0;
}

int drv_adc_software_trigger(jxc_handle_t handle, uint8_t sequence)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return -1;

    adc_software_trigger_enable(adc_handle->adc, sequence);

    return 0;
}

int drv_adc_read(jxc_handle_t handle, adc_channel_t *channel, uint16_t *data, uint32_t timeout)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle) || !channel || !data)
        return -1;

    drv_adc_routine_channel_config(handle, channel, 0, 1);
    adc_flag_clear(adc_handle->adc, ADC_FLAG_EOC);
    adc_software_trigger_enable(adc_handle->adc, ADC_ROUTINE_CHANNEL);

    while(adc_flag_get(adc_handle->adc, ADC_FLAG_EOC) == RESET){
        if(timeout-- == 0)
            return -2;
    }

    *data = adc_routine_data_read(adc_handle->adc);

    return 0;
}

uint16_t drv_adc_routine_data_read(jxc_handle_t handle)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return 0;

    return adc_routine_data_read(adc_handle->adc);
}

uint16_t drv_adc_inserted_data_read(jxc_handle_t handle, uint8_t inserted_channel)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle))
        return 0;

    return adc_inserted_data_read(adc_handle->adc, inserted_channel);
}

int drv_adc_read_dma(jxc_handle_t handle, uint16_t *data, uint32_t size)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle) || !data || size == 0 || !adc_handle->hw.dma.valid)
        return -1;

    adc_dma_mode_enable(adc_handle->adc);
    adc_dma_request_after_last_enable(adc_handle->adc);

    return drv_dma_update_start(&adc_handle->hw.dma, (uint8_t *)data, size);
}

int drv_adc_stop_dma(jxc_handle_t handle)
{
    drv_adc_handle_t *adc_handle = (drv_adc_handle_t *)handle;

    if(!drv_adc_is_valid(adc_handle) || !adc_handle->hw.dma.valid)
        return -1;

    return drv_dma_stop(&adc_handle->hw.dma);
}

void drv_adc_internal_channel_enable(uint32_t function)
{
    adc_channel_16_to_18(function, ENABLE);
}

void drv_adc_internal_channel_disable(uint32_t function)
{
    adc_channel_16_to_18(function, DISABLE);
}
