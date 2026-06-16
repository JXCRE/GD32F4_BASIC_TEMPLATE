#ifndef __FIFO_H
#define __FIFO_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif 

#ifndef jxc_handle_t
typedef void* jxc_handle_t;
#endif

jxc_handle_t fifo_create(uint32_t size);
int fifo_destroy(jxc_handle_t handle);
uint32_t fifo_put(jxc_handle_t handle, uint8_t *data, uint32_t len);
uint32_t fifo_get(jxc_handle_t handle, uint8_t *data, uint32_t len);
uint32_t fifo_count(jxc_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* __FIFO_H */
