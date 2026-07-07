#ifndef __DRV_DMA_H__
#define __DRV_DMA_H__

#include <stdint.h>
#include <stdbool.h>
#include "gd32f4xx.h"

/**
 * 实现DMA外设到内存、内存到外设，支持DMA通道、DMA子外设的配置
 * 
 */

typedef struct{
    bool                    valid;
    uint32_t                periph;      // DMA0/DMA1
    dma_channel_enum        channel;     // DMA_CHx
    dma_subperipheral_enum  sub_periph;  // DMA_SUBPERIx
}dma_info_t;

#define DMA_INFO(periph_a, channel_a, sub_periph_a, valid_a) \
    ((dma_info_t){ \
        .periph = periph_a, \
        .channel = channel_a, \
        .sub_periph = sub_periph_a, \
        .valid = valid_a \
    })

int drv_dma_tx_init(dma_info_t *dma_info, uint32_t dst_addr);
int drv_dma_rx_init(dma_info_t *dma_info, uint32_t src_addr);
int drv_dma_stop(dma_info_t *dma_info);
int drv_dma_start(dma_info_t *dma_info);
int drv_dma_mem_update(dma_info_t *dma_info, uint8_t *addr, uint32_t size);
int drv_dma_update_start(dma_info_t *dma_info, uint8_t *addr, uint32_t size);

uint32_t drv_dma_number_get(dma_info_t *dma_info);
FlagStatus drv_dma_flag_get(dma_info_t *dma_info, uint32_t flag);
void drv_dma_flag_clear(dma_info_t *dma_info, uint32_t flag);
FlagStatus drv_dma_interrupt_flag_get(dma_info_t *dma_info, uint32_t interrupt);
void drv_dma_interrupt_flag_clear(dma_info_t *dma_info, uint32_t interrupt);

#endif
