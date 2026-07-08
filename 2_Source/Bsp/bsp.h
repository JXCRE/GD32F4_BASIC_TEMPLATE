#ifndef __BSP_H
#define __BSP_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "gd32f4xx.h"

#ifdef __cplusplus
 extern "C" {
#endif 

extern bool _1ms_flag;
extern bool _10ms_flag;
extern bool _20ms_flag;
extern bool _500ms_flag;

//--------------------------------------------------VERSION
#define MAIN_VERSION    0                       //主版本号
#define SUB_VERSION     1                       //次版本号
#define FIX_VERSION     0                       //修复版本号
//--------------------------------------------------SYS
void bsp_systick_init(void);                    //初始化系统定时器
void bsp_nvic_priority_group_set(uint32_t nvic_prigroup);
void bsp_tick_update(void);                     //更新系统tick(系统中断中调用)
void bsp_delay_1ms(uint32_t ms);                //延时1ms
void bsp_delay_1us(uint32_t us);                //延时1us(如果us值过大，可能延时异常)
uint32_t bsp_get_sys_msTime(void);              //返回sys运行时间，ms为单位
//--------------------------------------------------COMPILE DATE
void get_compile_date(uint16_t *year, uint8_t *month, uint8_t *day);
void get_compile_time(uint8_t *hour, uint8_t *minute, uint8_t *second);
void get_board_version(uint8_t *m_version, uint8_t *s_version, uint8_t *f_version);
//--------------------------------------------------





#ifdef __cplusplus
}
#endif

#endif
