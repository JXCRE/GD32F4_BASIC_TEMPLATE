#include "bsp.h"

//--------------------------------------------------SYS
volatile static uint32_t delay;
volatile static uint32_t sys_tick = 0;
#define SYSTICK     10000   //HZ
#define _1MS_T      (uint16_t)(0.001*SYSTICK)
#define _10MS_T     (uint16_t)(0.01*SYSTICK)
#define _20MS_T     (uint16_t)(0.02*SYSTICK)
#define _500MS_T    (uint16_t)(0.5*SYSTICK)
static uint16_t _1ms_t = 0;
static uint16_t _10ms_t = 0;
static uint16_t _20ms_t = 0;
static uint16_t _500ms_t = 0;
bool _1ms_flag = false;
bool _10ms_flag = false;
bool _20ms_flag = false;
bool _500ms_flag = false;
//--------------------------------------------------



//----------------------------------------------------------------------------------------------------SYS
void bsp_systick_init(void)
{
    /* setup systick timer for systickHz interrupts */
    if(SysTick_Config(SystemCoreClock / SYSTICK)){
        while(1){}
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

void bsp_tick_update(void)
{
    sys_tick++;

    if(++_1ms_t     >= _1MS_T){     _1ms_t = 0;     _1ms_flag = true;}
    if(++_10ms_t    >= _10MS_T){    _10ms_t = 0;    _10ms_flag = true;}
    if(++_20ms_t    >= _20MS_T){    _20ms_t = 0;    _20ms_flag = true;}
    if(++_500ms_t   >= _500MS_T){   _500ms_t = 0;   _500ms_flag = true;}

    if(0U != delay){
        delay--;
    }
}

void bsp_delay_1ms(uint32_t ms)
{
    delay = ms*(0.001*SYSTICK);

    while(0U != delay){}
}

void bsp_delay_1us(uint32_t us)
{
    uint32_t ticks;
    uint32_t told, tnow, reload;
    uint32_t tcnt = 0;

    //确保SysTick定时器已启用
    if((SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) == 0){
        return;//未启用退出
    }
    
    ticks = us * (SystemCoreClock / 1000000);   //计算需要的时钟周期
    reload = SysTick->LOAD;                     //获取systick的重装载值
    told = SysTick->VAL;                        //记录开始时的计数值

    while(1)
    {
        tnow = SysTick->VAL;
        // 如果当前值不等于上次记录的值，说明计数器在递减（时间在流逝）
        if(tnow != told){
            if(tnow < told){
                tcnt += told - tnow;            //未发生重载 (tnow < told)
            }else{
                tcnt += told + reload - tnow;   //发生了重载 (tnow > told)
            }
 
            // 更新上次记录值，为下一次比较做准备
            told = tnow;
 
            // 判断累计周期数是否达到目标
            if(tcnt >= ticks){
                return;
            }
        }
    }
}

uint32_t bsp_get_sys_msTime(void)
{
    return sys_tick/(0.001*SYSTICK);
}

//----------------------------------------------------------------------------------------------------COMPILE DATE
void get_compile_date(uint16_t *year, uint8_t *month, uint8_t *day)
{
    const char *pMonth[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const char date[12] = __DATE__;

    for(int i=0; i<12; i++){
        if(memcmp(date, pMonth[i], 3) == 0){
            *month = i+1;
            break;
        }
    }
    *year = (uint16_t)atoi(date+9);
    *day = (uint8_t)atoi(date+4);
}

void get_compile_time(uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    const char time[12] = __TIME__;
    *hour = (uint8_t)atoi(time);
    *minute = (uint8_t)atoi(time+3);
    *second = (uint8_t)atoi(time+6);
}

void get_board_version(uint8_t *m_version, uint8_t *s_version, uint8_t *f_version)
{
    *m_version = MAIN_VERSION;
    *s_version = SUB_VERSION;
    *f_version = FIX_VERSION;
    return;
}
