#include "drv_dac.h"
#include <stdlib.h>
#include <string.h>

typedef struct{
    char name[10];
    uint32_t dac;
    dac_hardware_t hw;
}drv_dac_handle_t;

static bool drv_dac_is_valid(drv_dac_handle_t *dac_handle)
{
    return (dac_handle && dac_handle->dac == DAC0);
}

static bool drv_dac_out_is_valid(uint8_t out)
{
    return (out == DAC_OUT0 || out == DAC_OUT1);
}

static dma_info_t *drv_dac_dma_get(drv_dac_handle_t *dac_handle, uint8_t out)
{
    if(out == DAC_OUT0)
        return &dac_handle->hw.out0_dma;

    return &dac_handle->hw.out1_dma;
}

static uint32_t drv_dac_data_reg_get(uint32_t dac, uint8_t out, uint32_t align)
{
    if(out == DAC_OUT0){
        if(align == DAC_ALIGN_12B_L)
            return (uint32_t)&DAC_OUT0_L12DH(dac);
        if(align == DAC_ALIGN_8B_R)
            return (uint32_t)&DAC_OUT0_R8DH(dac);

        return (uint32_t)&DAC_OUT0_R12DH(dac);
    }

    if(align == DAC_ALIGN_12B_L)
        return (uint32_t)&DAC_OUT1_L12DH(dac);
    if(align == DAC_ALIGN_8B_R)
        return (uint32_t)&DAC_OUT1_R8DH(dac);

    return (uint32_t)&DAC_OUT1_R12DH(dac);
}

static void drv_dac_gpio_init(drv_dac_handle_t *dac_handle)
{
    if(dac_handle->hw.out0_valid){
        dac_handle->hw.out0.mode = GPIO_MODE_ANALOG;
        dac_handle->hw.out0.pupd = GPIO_PUPD_NONE;
        dac_handle->hw.out0.otype = GPIO_OTYPE_PP;
        dac_handle->hw.out0.func = GPIO_AF_0;
        drv_gpio_init(&dac_handle->hw.out0);
    }
    if(dac_handle->hw.out1_valid){
        dac_handle->hw.out1.mode = GPIO_MODE_ANALOG;
        dac_handle->hw.out1.pupd = GPIO_PUPD_NONE;
        dac_handle->hw.out1.otype = GPIO_OTYPE_PP;
        dac_handle->hw.out1.func = GPIO_AF_0;
        drv_gpio_init(&dac_handle->hw.out1);
    }
}

jxc_handle_t drv_dac_create(char *name, uint32_t dac)
{
    drv_dac_handle_t *dac_handle;

    dac_handle = (drv_dac_handle_t *)malloc(sizeof(drv_dac_handle_t));
    if(!dac_handle)
        return NULL;

    memset(dac_handle, 0, sizeof(drv_dac_handle_t));
    if(name)
        strncpy(dac_handle->name, name, sizeof(dac_handle->name) - 1);
    dac_handle->dac = dac;

    return (jxc_handle_t)dac_handle;
}

void drv_dac_destroy(jxc_handle_t handle)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!dac_handle)
        return;

    dac_disable(dac_handle->dac, DAC_OUT0);
    dac_disable(dac_handle->dac, DAC_OUT1);
    free(dac_handle);
}

int drv_dac_hw_init(jxc_handle_t handle, dac_hardware_t *hw)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle) || !hw)
        return -1;

    dac_handle->hw = *hw;
    rcu_periph_clock_enable(RCU_DAC);
    dac_deinit(dac_handle->dac);
    drv_dac_gpio_init(dac_handle);

    return 0;
}

int drv_dac_channel_init(jxc_handle_t handle, dac_channel_t *channel)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;
    dma_info_t *dma;

    if(!drv_dac_is_valid(dac_handle) || !channel || !drv_dac_out_is_valid(channel->out))
        return -1;

    if(channel->buffer_enable)
        dac_output_buffer_enable(dac_handle->dac, channel->out);
    else
        dac_output_buffer_disable(dac_handle->dac, channel->out);

    dac_trigger_source_config(dac_handle->dac, channel->out, channel->trigger_source);
    if(channel->trigger_enable)
        dac_trigger_enable(dac_handle->dac, channel->out);
    else
        dac_trigger_disable(dac_handle->dac, channel->out);

    dac_wave_mode_config(dac_handle->dac, channel->out, channel->wave_mode);
    if(channel->wave_mode == DAC_WAVE_MODE_LFSR)
        dac_lfsr_noise_config(dac_handle->dac, channel->out, channel->wave_bits);
    else if(channel->wave_mode == DAC_WAVE_MODE_TRIANGLE)
        dac_triangle_noise_config(dac_handle->dac, channel->out, channel->wave_bits);

    dma = drv_dac_dma_get(dac_handle, channel->out);
    if(channel->dma_enable && dma->valid){
        dac_dma_enable(dac_handle->dac, channel->out);
    }else{
        dac_dma_disable(dac_handle->dac, channel->out);
    }

    dac_enable(dac_handle->dac, channel->out);

    return 0;
}

int drv_dac_enable_irq(jxc_handle_t handle, uint32_t interrupt, uint8_t pre_priority, uint8_t sub_priority)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle))
        return -1;

    dac_interrupt_enable(dac_handle->dac, interrupt);
    nvic_irq_enable(TIMER5_DAC_IRQn, pre_priority, sub_priority);

    return 0;
}

int drv_dac_disable_irq(jxc_handle_t handle, uint32_t interrupt)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle))
        return -1;

    dac_interrupt_disable(dac_handle->dac, interrupt);
    nvic_irq_disable(TIMER5_DAC_IRQn);

    return 0;
}

FlagStatus drv_dac_interrupt_flag_get(jxc_handle_t handle, uint32_t flag)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle))
        return RESET;

    return dac_interrupt_flag_get(dac_handle->dac, flag);
}

void drv_dac_interrupt_flag_clear(jxc_handle_t handle, uint32_t flag)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle))
        return;

    dac_interrupt_flag_clear(dac_handle->dac, flag);
}

FlagStatus drv_dac_flag_get(jxc_handle_t handle, uint32_t flag)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle))
        return RESET;

    return dac_flag_get(dac_handle->dac, flag);
}

void drv_dac_flag_clear(jxc_handle_t handle, uint32_t flag)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle))
        return;

    dac_flag_clear(dac_handle->dac, flag);
}

int drv_dac_enable(jxc_handle_t handle, uint8_t out)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle) || !drv_dac_out_is_valid(out))
        return -1;

    dac_enable(dac_handle->dac, out);

    return 0;
}

int drv_dac_disable(jxc_handle_t handle, uint8_t out)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle) || !drv_dac_out_is_valid(out))
        return -1;

    dac_disable(dac_handle->dac, out);

    return 0;
}

int drv_dac_data_set(jxc_handle_t handle, uint8_t out, uint32_t align, uint16_t data)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle) || !drv_dac_out_is_valid(out))
        return -1;

    dac_data_set(dac_handle->dac, out, align, data);

    return 0;
}

uint16_t drv_dac_output_get(jxc_handle_t handle, uint8_t out)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle) || !drv_dac_out_is_valid(out))
        return 0;

    return dac_output_value_get(dac_handle->dac, out);
}

int drv_dac_software_trigger(jxc_handle_t handle, uint8_t out)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;

    if(!drv_dac_is_valid(dac_handle) || !drv_dac_out_is_valid(out))
        return -1;

    dac_software_trigger_enable(dac_handle->dac, out);

    return 0;
}

int drv_dac_write_dma(jxc_handle_t handle, uint8_t out, uint32_t align, uint8_t *data, uint32_t size)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;
    dma_info_t *dma;

    if(!drv_dac_is_valid(dac_handle) || !drv_dac_out_is_valid(out) || !data || size == 0)
        return -1;

    dma = drv_dac_dma_get(dac_handle, out);
    if(!dma->valid)
        return -1;

    dma->periph_memory_width = (align == DAC_ALIGN_8B_R) ? DMA_PERIPH_WIDTH_8BIT : DMA_PERIPH_WIDTH_16BIT;
    drv_dma_tx_init(dma, drv_dac_data_reg_get(dac_handle->dac, out, align));
    dac_dma_enable(dac_handle->dac, out);

    return drv_dma_update_start(dma, data, size);
}

int drv_dac_stop_dma(jxc_handle_t handle, uint8_t out)
{
    drv_dac_handle_t *dac_handle = (drv_dac_handle_t *)handle;
    dma_info_t *dma;

    if(!drv_dac_is_valid(dac_handle) || !drv_dac_out_is_valid(out))
        return -1;

    dma = drv_dac_dma_get(dac_handle, out);
    if(!dma->valid)
        return -1;

    return drv_dma_stop(dma);
}
