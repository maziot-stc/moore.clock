#include "reg52.h"
#include "main.h"
#include "bsp_delay.h"
#include "bsp_key.h"

#define Segment P0
#define Digital P1

/* 定义模型变量 */
u8 value[6] = {1, 1, 5, 9, 0, 0};                                               // 记录数码管每一位对应值的数组
u8 alarm[4] = {1, 2, 0, 0};                                                     // 记录闹钟的时间的变量，这里只支持一个闹钟
u8 location = 0;                                                                // 记录当期是哪一位闪烁
u8 state = STATE_RUNNING;                                                       // 记录当前电子表的状态

sbit beep = P3^7;

/* 数码管段选码、位选码 */
u8 SegCode[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};	// 段选高电平有效
u8 DigCode[6] = {0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf};				// 位选低电平有效

u8 InterruptCount = 0;

int main()
{
        u8 i, j;
        
        /* 这里设置的值是表示，10次扫描全部显示，10次扫描location位不显示 */
        u8 dispalyCount = 10;
        
        IT0 = 1;
        TMOD = 0x01;
        TH0 = 0x0b;
        TL0 = 0xdc;
        IE = 0x83;
        TR0 = 1;
        
        beep = 0;
        
        while(1)
        {
                if((state == STATE_RUNNING) || (state == STATE_SETTING))
                {
                        
                        /* 这里扫描的时候是正常显示6位 */
                        for(j = dispalyCount; j > 0; j--)
                        {
                                for(i = 0; i < 6; i++)
                                {
                                        Digital = DigCode[i];
                                        
                                        /* 这是是在第一位和第三位的位置上添加小数点 */
                                        if(i == 1 || i == 3)
                                        {
                                                /* 通过按位与操作，在不改变低7位的前提下，将位7置为高电平 */
                                                Segment = SegCode[value[i]] | 0x80;
                                        }
                                        else
                                        {
                                                Segment = SegCode[value[i]];
                                        }
                                        delay_ms(1);
                                }
                        }
                        
                        if(state == STATE_SETTING)
                        {
                                /* 这里扫描的时候location对应位不显示，只显示其他位 */
                                for(j = dispalyCount; j > 0; j--)
                                {
                                        for(i = 0; i < 6; i++)
                                        {
                                                /* 判断当期扫描的位是不是location对应位 */
                                                if(location == i)
                                                {
                                                        /* 如果是的话，不选中任何位选，也就是全灭 */
                                                        Digital = 0xff;
                                                }
                                                else
                                                {
                                                        /* 对于其他为就正常显示就好 */
                                                        Digital = DigCode[i];
                                                }
                                                
                                                /* 这里和上面是一样的，加小数点的操作 */
                                                if(i == 1 || i == 3)
                                                {
                                                        Segment = SegCode[value[i]] | 0x80;
                                                }
                                                else
                                                {
                                                        Segment = SegCode[value[i]];
                                                }
                                                delay_ms(1);
                                        }
                                }
                        }
                }
                else if(state == STATE_ALARM)
                {
                        /* 这里扫描的时候是正常显示4位 */
                        for(j = dispalyCount; j > 0; j--)
                        {
                                for(i = 0; i < 4; i++)
                                {
                                        Digital = DigCode[i];
                                        
                                        /* 这是是在第一位添加小数点 */
                                        if(i == 1)
                                        {
                                                /* 通过按位与操作，在不改变低7位的前提下，将位7置为高电平 */
                                                Segment = SegCode[alarm[i]] | 0x80;
                                        }
                                        else
                                        {
                                                Segment = SegCode[alarm[i]];
                                        }
                                        delay_ms(1);
                                }
                        }
                        
                        for(j = dispalyCount; j > 0; j--)
                        {
                                for(i = 0; i < 4; i++)
                                {
                                        /* 判断当期扫描的位是不是location对应位 */
                                        if(location == i)
                                        {
                                                /* 如果是的话，不选中任何位选，也就是全灭 */
                                                Digital = 0xff;
                                        }
                                        else
                                        {
                                                /* 对于其他为就正常显示就好 */
                                                Digital = DigCode[i];
                                        }
                                        
                                        /* 这里和上面是一样的，加小数点的操作 */
                                        if(i == 1)
                                        {
                                                Segment = SegCode[alarm[i]] | 0x80;
                                        }
                                        else
                                        {
                                                Segment = SegCode[alarm[i]];
                                        }
                                        delay_ms(1);
                                }
                        }
                }
        }
}

/**
 * @Descroption 外部中断0中断服务函数
 */ 
void INT0_Handler() interrupt 0
{
        Key_Control(Key_Scan());
}

/**
 * @Descroption 定时器0中断服务函数
 */
void TIM0_Handler() interrupt 1
{
        /* 手动的装载初值 */
        TH0 = 0x0b;
        TL0 = 0xdc;
        
        InterruptCount++;
        
        if(InterruptCount >= 16)
        {
                InterruptCount = 0;
                Run();
        }        
}

/**
 * @Descroption 电子表正常运行函数，每1s加一，并完成的对应的进位
 */ 
void Run(void)
{     
        if(value[5] < 9)
        {
                value[5]++;
        }
        else
        {
                value[5] = 0;
                
                if(value[4] < 5)
                {
                        value[4]++;
                }
                else
                {
                        value[4] = 0;
                        
                        if(value[3] < 9)
                        {
                                value[3]++;
                        }
                        else
                        {
                                value[3] = 0;
                                
                                if(value[2] < 5)
                                {
                                        value[2]++;
                                }
                                else
                                {
                                        value[2] = 0;
                                        
                                        /* 如果时的十位为0或者1的话，时的个位其实是可以显示0-9的 */
                                        /* 如果时的十位为2的话，那时的个位就只能显示0-3了 */
                                        if(((value[0] < 2) && (value[1] < 9)) || ((value[0] == 2) && (value[1] < 3)))
                                        {
                                                value[1]++;
                                        }
                                        else 
                                        {
                                                value[1] = 0;
                                                
                                                if(value[0] < 2)
                                                {
                                                        value[0]++;
                                                }
                                                else
                                                {
                                                        value[0] = 0;
                                                }
                                        }
                                }
                        }
                }
        }
        
        if((value[0] == alarm[0]) && (value[1] == alarm[1]) && (value[2] == alarm[2]) && (value[3] == alarm[3]))
        {
                beep = 1;
        }
        else
        {
                beep = 0;
        }
}
