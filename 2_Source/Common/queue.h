#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef jxc_handle_t
typedef void* jxc_handle_t;
#endif

jxc_handle_t queue_create(uint32_t size, uint32_t itemSize);
int queue_destroy(jxc_handle_t handle);

uint32_t queue_put(jxc_handle_t handle, uint8_t *data);
uint32_t queue_get(jxc_handle_t handle, uint8_t *data);
uint32_t queue_get_latest(jxc_handle_t handle, uint8_t *data);

uint32_t queue_get_count(jxc_handle_t handle);
uint32_t queue_del(jxc_handle_t handle, uint32_t num);

#ifdef __cplusplus
}
#endif

#endif
