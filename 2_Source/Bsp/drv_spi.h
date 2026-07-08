#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include <stdint.h>
#include <stdbool.h>
#include "gd32f4xx.h"
#include "drv_gpio.h"
#include "drv_dma.h"

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef jxc_handle_t
typedef void* jxc_handle_t;
#endif

typedef spi_parameter_struct spi_params_t;

typedef struct{
    pin_info_t sck;
    pin_info_t miso;
    pin_info_t mosi;
    pin_info_t nss;
    bool nss_valid;
    dma_info_t tx_dma;
    dma_info_t rx_dma;
}spi_hardware_t;

jxc_handle_t drv_spi_create(char *name, uint32_t spi);
void drv_spi_destroy(jxc_handle_t handle);

int drv_spi_hw_init(jxc_handle_t handle, spi_hardware_t *hw, spi_params_t *params);
int drv_spi_enable_irq(jxc_handle_t handle, uint8_t interrupt, uint8_t pre_priority, uint8_t sub_priority);
int drv_spi_disable_irq(jxc_handle_t handle, uint8_t interrupt);
FlagStatus drv_spi_interrupt_flag_get(jxc_handle_t handle, uint8_t flag);
FlagStatus drv_spi_flag_get(jxc_handle_t handle, uint32_t flag);

int drv_spi_write_read(jxc_handle_t handle, uint8_t *tx_data, uint8_t *rx_data, uint32_t size, uint32_t timeout);
int drv_spi_write(jxc_handle_t handle, uint8_t *data, uint32_t size, uint32_t timeout);
int drv_spi_read(jxc_handle_t handle, uint8_t *data, uint32_t size, uint8_t dummy, uint32_t timeout);
int drv_spi_write_dma(jxc_handle_t handle, uint8_t *data, uint32_t size);
int drv_spi_read_dma(jxc_handle_t handle, uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif
