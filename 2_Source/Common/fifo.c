#include "fifo.h"
#include <stdlib.h>
#include <string.h>

typedef struct{
    uint32_t    maxSize;
    uint8_t     *controlData;
    uint32_t    writeIndex;
    uint32_t    readIndex;
    uint32_t    controlSize;
}fifo_handle_t;

jxc_handle_t fifo_create(uint32_t size)
{
    if(size == 0)
        return NULL;

    fifo_handle_t *hndl = (fifo_handle_t *)malloc(sizeof(fifo_handle_t));
    if(!hndl)
        return NULL;
    hndl->maxSize = size;
    hndl->controlData = (uint8_t *)malloc(sizeof(uint8_t)*hndl->maxSize);
    if(!hndl->controlData){
        free(hndl);
        return NULL;
    }

    hndl->writeIndex = 0;
    hndl->readIndex = 0;
    hndl->controlSize = 0;

    memset(hndl->controlData, 0, hndl->maxSize);

    return hndl;
}

int fifo_destroy(jxc_handle_t handle)
{
    fifo_handle_t *hndl = (fifo_handle_t *)handle;
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

uint32_t fifo_put(jxc_handle_t handle, uint8_t *data, uint32_t len)
{
    fifo_handle_t *hndl = (fifo_handle_t *)handle;

    if(!hndl || (!data && len > 0) || len == 0)
        return 0;

    //这里在fifo满了的时候，没有更新controlSize以及readIndex,可能导致错位
    //但是覆盖在我们的设计中是不允许的操作

    if(len > hndl->maxSize - hndl->controlSize)
        return 0;

    if(len <= hndl->maxSize - hndl->writeIndex){
        memcpy(hndl->controlData + hndl->writeIndex, data, len);
    }else{
        uint32_t tmpSize = hndl->maxSize - hndl->writeIndex;
        memcpy(hndl->controlData + hndl->writeIndex, data, tmpSize);
        memcpy(hndl->controlData, data + tmpSize, len - tmpSize);
    }
    hndl->writeIndex = (hndl->writeIndex + len) % hndl->maxSize;
    hndl->controlSize += len;

    return len;
}

uint32_t fifo_get(jxc_handle_t handle, uint8_t *data, uint32_t len)
{
    fifo_handle_t *hndl = (fifo_handle_t *)handle;

    if(!hndl || (!data && len > 0) || len == 0)
        return 0;

    if(len > hndl->controlSize)
        return 0;

    if(len <= hndl->maxSize - hndl->readIndex){
        memcpy(data, hndl->controlData + hndl->readIndex, len);
    }else{
        uint32_t tmpSize = hndl->maxSize - hndl->readIndex;
        memcpy(data, hndl->controlData + hndl->readIndex, tmpSize);
        memcpy(data + tmpSize, hndl->controlData, len - tmpSize);
    }
    hndl->readIndex = (hndl->readIndex + len)%hndl->maxSize;
    hndl->controlSize -= len;

    return len;
}

uint32_t fifo_get_count(jxc_handle_t handle)
{
    fifo_handle_t *hndl = (fifo_handle_t *)handle;

    if(!hndl)
        return 0;

    return hndl->controlSize;
}

uint32_t fifo_del(jxc_handle_t handle, uint32_t num)
{
    fifo_handle_t *hndl = (fifo_handle_t *)handle;

    if(!hndl)
        return 0;

    for(uint32_t i = 0; i < num; i++){
        if(hndl->controlSize > 0){
            hndl->controlData[hndl->readIndex] = 0;//删除数据置零
            hndl->readIndex = (hndl->readIndex + 1) % hndl->maxSize;
            hndl->controlSize -= 1;
        }else{
            break;
        }
    }

    return 0;
}


