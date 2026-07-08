#include "drv_spi.h"
#include "bsp.h"
#include <stdlib.h>
#include <string.h>

typedef struct{
    spi_hardware_t hw;
    spi_params_t params;
    uint32_t spi;
    char name[10];
}spi_handle_t;

static void drv_spi_clock_enable(uint32_t spi)
{
    switch(spi){
        case SPI0:
            rcu_periph_clock_enable(RCU_SPI0);
            break;
        case SPI1:
            rcu_periph_clock_enable(RCU_SPI1);
            break;
        case SPI2:
            rcu_periph_clock_enable(RCU_SPI2);
            break;
        case SPI3:
            rcu_periph_clock_enable(RCU_SPI3);
            break;
        case SPI4:
            rcu_periph_clock_enable(RCU_SPI4);
            break;
        case SPI5:
            rcu_periph_clock_enable(RCU_SPI5);
            break;
        default:
            break;
    }
}

static IRQn_Type drv_spi_irq_get(uint32_t spi)
{
    switch(spi){
        case SPI0:
            return SPI0_IRQn;
        case SPI1:
            return SPI1_IRQn;
        case SPI2:
            return SPI2_IRQn;
#if defined(GD32F450) || defined(GD32F470)
        case SPI3:
            return SPI3_IRQn;
        case SPI4:
            return SPI4_IRQn;
        case SPI5:
            return SPI5_IRQn;
#endif
        default:
            return SPI0_IRQn;
    }
}

static int drv_spi_wait_flag(uint32_t spi, uint32_t flag, FlagStatus status, uint32_t timeout)
{
    uint32_t start = bsp_get_sys_msTime();

    while(status != spi_i2s_flag_get(spi, flag)){
        if((bsp_get_sys_msTime() - start) > timeout)
            return -1;
    }

    return 0;
}

jxc_handle_t drv_spi_create(char *name, uint32_t spi)
{
    spi_handle_t *hndl;

    hndl = (spi_handle_t *)malloc(sizeof(spi_handle_t));
    if(!hndl)
        return NULL;

    memset(hndl, 0, sizeof(spi_handle_t));
    if(name){
        strncpy(hndl->name, name, sizeof(hndl->name) - 1);
    }

    hndl->spi = spi;

    return hndl;
}

void drv_spi_destroy(jxc_handle_t handle)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl)
        return;

    free(hndl);
}

int drv_spi_hw_init(jxc_handle_t handle, spi_hardware_t *hw, spi_params_t *params)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl || !hw || !params)
        return -1;

    hndl->hw = *hw;
    hndl->params = *params;

    drv_spi_clock_enable(hndl->spi);

    drv_gpio_init(&hndl->hw.sck);
    drv_gpio_init(&hndl->hw.miso);
    drv_gpio_init(&hndl->hw.mosi);
    if(hndl->hw.nss_valid){
        drv_gpio_init(&hndl->hw.nss);
    }

    spi_i2s_deinit(hndl->spi);
    spi_init(hndl->spi, &hndl->params);
    if(hndl->params.nss == SPI_NSS_HARD){
        spi_nss_output_enable(hndl->spi);
    }else{
        spi_nss_internal_high(hndl->spi);
    }

    if(hndl->hw.tx_dma.valid){
        spi_dma_enable(hndl->spi, SPI_DMA_TRANSMIT);
        drv_dma_tx_init(&hndl->hw.tx_dma, (uint32_t)&SPI_DATA(hndl->spi));
    }

    if(hndl->hw.rx_dma.valid){
        spi_dma_enable(hndl->spi, SPI_DMA_RECEIVE);
        drv_dma_rx_init(&hndl->hw.rx_dma, (uint32_t)&SPI_DATA(hndl->spi));
    }

    spi_enable(hndl->spi);

    return 0;
}

int drv_spi_enable_irq(jxc_handle_t handle, uint8_t interrupt, uint8_t pre_priority, uint8_t sub_priority)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl)
        return -1;

    spi_i2s_interrupt_enable(hndl->spi, interrupt);
    nvic_irq_enable(drv_spi_irq_get(hndl->spi), pre_priority, sub_priority);

    return 0;
}

int drv_spi_disable_irq(jxc_handle_t handle, uint8_t interrupt)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl)
        return -1;

    spi_i2s_interrupt_disable(hndl->spi, interrupt);
    nvic_irq_disable(drv_spi_irq_get(hndl->spi));

    return 0;
}

FlagStatus drv_spi_interrupt_flag_get(jxc_handle_t handle, uint8_t flag)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl)
        return RESET;

    return spi_i2s_interrupt_flag_get(hndl->spi, flag);
}

FlagStatus drv_spi_flag_get(jxc_handle_t handle, uint32_t flag)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl)
        return RESET;

    return spi_i2s_flag_get(hndl->spi, flag);
}

int drv_spi_write_read(jxc_handle_t handle, uint8_t *tx_data, uint8_t *rx_data, uint32_t size, uint32_t timeout)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl || size == 0 || (!tx_data && !rx_data))
        return -1;

    for(uint32_t i = 0; i < size; i++){
        if(drv_spi_wait_flag(hndl->spi, SPI_FLAG_TBE, SET, timeout) != 0)
            return -2;

        spi_i2s_data_transmit(hndl->spi, tx_data ? tx_data[i] : 0xFFU);

        if(drv_spi_wait_flag(hndl->spi, SPI_FLAG_RBNE, SET, timeout) != 0)
            return -2;

        if(rx_data){
            rx_data[i] = (uint8_t)spi_i2s_data_receive(hndl->spi);
        }else{
            (void)spi_i2s_data_receive(hndl->spi);
        }
    }

    if(drv_spi_wait_flag(hndl->spi, SPI_FLAG_TRANS, RESET, timeout) != 0)
        return -2;

    return 0;
}

int drv_spi_write(jxc_handle_t handle, uint8_t *data, uint32_t size, uint32_t timeout)
{
    if(!data)
        return -1;

    return drv_spi_write_read(handle, data, NULL, size, timeout);
}

int drv_spi_read(jxc_handle_t handle, uint8_t *data, uint32_t size, uint8_t dummy, uint32_t timeout)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl || !data || size == 0)
        return -1;

    for(uint32_t i = 0; i < size; i++){
        if(drv_spi_wait_flag(hndl->spi, SPI_FLAG_TBE, SET, timeout) != 0)
            return -2;

        spi_i2s_data_transmit(hndl->spi, dummy);

        if(drv_spi_wait_flag(hndl->spi, SPI_FLAG_RBNE, SET, timeout) != 0)
            return -2;

        data[i] = (uint8_t)spi_i2s_data_receive(hndl->spi);
    }

    if(drv_spi_wait_flag(hndl->spi, SPI_FLAG_TRANS, RESET, timeout) != 0)
        return -2;

    return 0;
}

int drv_spi_write_dma(jxc_handle_t handle, uint8_t *data, uint32_t size)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl || !data || size == 0)
        return -1;

    return drv_dma_update_start(&hndl->hw.tx_dma, data, size);
}

int drv_spi_read_dma(jxc_handle_t handle, uint8_t *data, uint32_t size)
{
    spi_handle_t *hndl = (spi_handle_t *)handle;

    if(!hndl || !data || size == 0)
        return -1;

    return drv_dma_update_start(&hndl->hw.rx_dma, data, size);
}
