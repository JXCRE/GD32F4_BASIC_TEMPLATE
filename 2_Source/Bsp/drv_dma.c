#include "drv_dma.h"


static void drv_dma_clock_enable(uint32_t periph)
{
    if(periph == DMA0){
        rcu_periph_clock_enable(RCU_DMA0);
    }else{
        rcu_periph_clock_enable(RCU_DMA1);
    }
}

static bool drv_dma_is_valid(dma_info_t *dma_info)
{
    return (dma_info && dma_info->valid);
}

int drv_dma_tx_init(dma_info_t *dma_info, uint32_t dst_addr)
{
    dma_single_data_parameter_struct dma_init;

    if(!drv_dma_is_valid(dma_info))
        return -1;

    drv_dma_clock_enable(dma_info->periph);

    dma_channel_disable(dma_info->periph, dma_info->channel);
    dma_deinit(dma_info->periph, dma_info->channel);

    dma_single_data_para_struct_init(&dma_init);
    dma_init.direction = DMA_MEMORY_TO_PERIPH;
    dma_init.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init.periph_addr = dst_addr;
    dma_init.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init.priority = DMA_PRIORITY_ULTRA_HIGH;

    dma_single_data_mode_init(dma_info->periph, dma_info->channel, &dma_init);
    dma_circulation_disable(dma_info->periph, dma_info->channel);
    dma_channel_subperipheral_select(dma_info->periph, dma_info->channel, dma_info->sub_periph);

    drv_dma_stop(dma_info);

    return 0;
}

int drv_dma_rx_init(dma_info_t *dma_info, uint32_t src_addr)
{
    dma_single_data_parameter_struct dma_init;

    if(!drv_dma_is_valid(dma_info))
        return -1;

    drv_dma_clock_enable(dma_info->periph);

    dma_channel_disable(dma_info->periph, dma_info->channel);
    dma_deinit(dma_info->periph, dma_info->channel);

    dma_single_data_para_struct_init(&dma_init);
    dma_init.direction = DMA_PERIPH_TO_MEMORY;
    dma_init.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init.periph_addr = src_addr;
    dma_init.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init.priority = DMA_PRIORITY_ULTRA_HIGH;

    dma_single_data_mode_init(dma_info->periph, dma_info->channel, &dma_init);
    dma_circulation_disable(dma_info->periph, dma_info->channel);
    dma_channel_subperipheral_select(dma_info->periph, dma_info->channel, dma_info->sub_periph);

    drv_dma_stop(dma_info);

    return 0;
}

int drv_dma_stop(dma_info_t *dma_info)
{
    if(!drv_dma_is_valid(dma_info))
        return -1;

    dma_channel_disable(dma_info->periph, dma_info->channel);
    dma_flag_clear(dma_info->periph, dma_info->channel, DMA_FLAG_FTF);
    dma_interrupt_flag_clear(dma_info->periph, dma_info->channel, DMA_INT_FLAG_FTF);

    return 0;
}

int drv_dma_start(dma_info_t *dma_info)
{
    if(!drv_dma_is_valid(dma_info))
        return -1;

    dma_channel_enable(dma_info->periph, dma_info->channel);

    return 0;
}

int drv_dma_mem_update(dma_info_t *dma_info, uint8_t *addr, uint32_t size)
{
    if(!drv_dma_is_valid(dma_info) || !addr || size == 0)
        return -1;

    dma_memory_address_config(dma_info->periph, dma_info->channel, DMA_MEMORY_0, (uint32_t)(uintptr_t)addr);
    dma_transfer_number_config(dma_info->periph, dma_info->channel, size);

    return 0;
}

int drv_dma_update_start(dma_info_t *dma_info, uint8_t *addr, uint32_t size)
{
    if(drv_dma_stop(dma_info) != 0)
        return -1;
    if(drv_dma_mem_update(dma_info, addr, size) != 0)
        return -1;
    if(drv_dma_start(dma_info) != 0)
        return -1;

    return 0;
}

uint32_t drv_dma_number_get(dma_info_t *dma_info)
{
    if(!drv_dma_is_valid(dma_info))
        return 0;

    return dma_transfer_number_get(dma_info->periph, dma_info->channel);
}

FlagStatus drv_dma_flag_get(dma_info_t *dma_info, uint32_t flag)
{
    if(!drv_dma_is_valid(dma_info))
        return RESET;

    return dma_flag_get(dma_info->periph, dma_info->channel, flag);
}

void drv_dma_flag_clear(dma_info_t *dma_info, uint32_t flag)
{
    if(!drv_dma_is_valid(dma_info))
        return;

    dma_flag_clear(dma_info->periph, dma_info->channel, flag);
}

FlagStatus drv_dma_interrupt_flag_get(dma_info_t *dma_info, uint32_t interrupt)
{
    if(!drv_dma_is_valid(dma_info))
        return RESET;

    return dma_interrupt_flag_get(dma_info->periph, dma_info->channel, interrupt);
}

void drv_dma_interrupt_flag_clear(dma_info_t *dma_info, uint32_t interrupt)
{
    if(!drv_dma_is_valid(dma_info))
        return;

    dma_interrupt_flag_clear(dma_info->periph, dma_info->channel, interrupt);
}
