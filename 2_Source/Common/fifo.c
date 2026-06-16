#include "fifo.h"

typedef struct{
    uint8_t *buffer;
    uint16_t size;
    uint16_t head;
    uint16_t tail;
    uint16_t count;
}fifo_t;

jxc_handle_t fifo_create(uint32_t size)
{

}

int fifo_destroy(jxc_handle_t handle)
{

}

uint32_t fifo_put(jxc_handle_t handle, uint8_t *data, uint32_t len)
{

}

uint32_t fifo_get(jxc_handle_t handle, uint8_t *data, uint32_t len)
{

}

uint32_t fifo_count(jxc_handle_t handle)
{
    
}
