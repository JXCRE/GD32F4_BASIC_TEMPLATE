#include "queue.h"
#include <stdlib.h>
#include <string.h>

typedef struct{
    uint32_t    maxSize;
    uint32_t    itemSize;
    uint8_t     *controlData;
    uint32_t    writeIndex;
    uint32_t    readIndex;
    uint32_t    controlSize;
}queue_handle_t;

jxc_handle_t queue_create(uint32_t size, uint32_t itemSize)
{
    if(size == 0 || itemSize == 0)
        return NULL;

    if(size > UINT32_MAX / itemSize)
        return NULL;

    queue_handle_t *hndl = (queue_handle_t *)malloc(sizeof(queue_handle_t));
    if(!hndl)
        return NULL;

    hndl->maxSize = size;
    hndl->itemSize = itemSize;
    hndl->controlData = (uint8_t *)malloc(hndl->maxSize * hndl->itemSize);
    if(!hndl->controlData){
        free(hndl);
        return NULL;
    }

    hndl->writeIndex = 0;
    hndl->readIndex = 0;
    hndl->controlSize = 0;

    memset(hndl->controlData, 0, hndl->maxSize * hndl->itemSize);

    return hndl;
}

int queue_destroy(jxc_handle_t handle)
{
    queue_handle_t *hndl = (queue_handle_t *)handle;
    if(hndl){
        if(hndl->controlData){
            free(hndl->controlData);
        }
        free(hndl);
    }else{
        return -1;
    }

    return 0;
}

uint32_t queue_put(jxc_handle_t handle, uint8_t *data)
{
    queue_handle_t *hndl = (queue_handle_t *)handle;

    if(!hndl || !data)
        return 0;

    if(hndl->controlSize >= hndl->maxSize)
        return 0;

    memcpy(hndl->controlData + hndl->writeIndex * hndl->itemSize, data, hndl->itemSize);
    hndl->writeIndex = (hndl->writeIndex + 1) % hndl->maxSize;
    hndl->controlSize += 1;

    return 1;
}

uint32_t queue_get(jxc_handle_t handle, uint8_t *data)
{
    queue_handle_t *hndl = (queue_handle_t *)handle;

    if(!hndl || !data)
        return 0;

    if(hndl->controlSize == 0)
        return 0;

    memcpy(data, hndl->controlData + hndl->readIndex * hndl->itemSize, hndl->itemSize);
    hndl->readIndex = (hndl->readIndex + 1) % hndl->maxSize;
    hndl->controlSize -= 1;

    return 1;
}

uint32_t queue_get_latest(jxc_handle_t handle, uint8_t *data)
{
    queue_handle_t *hndl = (queue_handle_t *)handle;
    uint32_t latestIndex;

    if(!hndl || !data)
        return 0;

    if(hndl->controlSize == 0)
        return 0;

    if(hndl->writeIndex == 0){
        latestIndex = hndl->maxSize - 1;
    }else{
        latestIndex = hndl->writeIndex - 1;
    }

    memcpy(data, hndl->controlData + latestIndex * hndl->itemSize, hndl->itemSize);
    memset(hndl->controlData, 0, hndl->maxSize * hndl->itemSize);
    hndl->writeIndex = 0;
    hndl->readIndex = 0;
    hndl->controlSize = 0;

    return 1;
}

uint32_t queue_get_count(jxc_handle_t handle)
{
    queue_handle_t *hndl = (queue_handle_t *)handle;

    if(!hndl)
        return 0;

    return hndl->controlSize;
}

uint32_t queue_del(jxc_handle_t handle, uint32_t num)
{
    queue_handle_t *hndl = (queue_handle_t *)handle;

    if(!hndl)
        return 0;

    for(uint32_t i = 0; i < num; i++){
        if(hndl->controlSize > 0){
            memset(hndl->controlData + hndl->readIndex * hndl->itemSize, 0, hndl->itemSize);
            hndl->readIndex = (hndl->readIndex + 1) % hndl->maxSize;
            hndl->controlSize -= 1;
        }else{
            break;
        }
    }

    return 0;
}
