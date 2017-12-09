#include "bsp_key.h"
#include "bsp_delay.h"

/** 
 * @Description 按键扫描函数(8个按键同时扫描)
 * @return u8   按键键值
 */
u8 Key_Scan(void)
{
        /* 保存按键键值的变量，默认值0xff表示没有按键按下 */
        u8 keyValue = 0xff;

        /* 用于松手检测的累加变量 */
        int i = 0;

        /* 这里在1ms前后检测两边是为了滤除按键抖动产生的尖峰脉冲 */
        if(GPIO_KEY != 0xff)
        {
                delay(1);

                /* 如果1ms前后检测都是低电平的话，就说明是真的有按键按下 */
                if(GPIO_KEY != 0xff)
                {
                        /* 真的有按键按下，则将按键键值存入keyValue中 */
                        keyValue = GPIO_KEY;

                        /* 这里是松手检测，在20ms内按键没有放开，程序会一直停在此处 */
                        /* 倘若50ms期间，松开了按键，则会跳出此while循环 */
                        /* 也就是说：按键没有松开的话，程序不会去做其他的事情 */
                        /* 当然也有松手检测的超时时间，就是我们设置的20ms */
                        while((i < 20) && (GPIO_KEY != 0xff))
                        {
                                delay(1);
                                i++;
                        }
                }
        }

        /* 返回按键的键值，通过解析这个键值，就能知道是哪一个按键按下 */
        return keyValue;
}

/** 
 * @Description 简单延时函数，它存在的目的是为了解决函数重入的问题
 */
void delay(u32 t)
{
        u32 i, j;
        for(i = t; i > 0; i--)
                for(j = 296; j > 0; j--);
}

/* 这里需要使用extern声明其他c文件定义的变量 */
extern u8 value[6];
extern u8 location;
extern u8 state;
extern u8 alarm[4];

/** 
 * @Description 按键控制函数(8个按键同时控制)
 * @param u8    按键扫描到的键值
 */
void Key_Control(u8 keyValue)
{
        /* 根据捕捉的键值解析出按下的键，并作出相应的处理 */
        switch(keyValue)
        {
                case(0xfe):     // 1111 1110 P2.0口对应按键按下 K0 数值减
                        if(state == STATE_SETTING)
                        {
                                if(value[location] > 0)
                                {
                                        value[location] -= 1;
                                }
                        }
                        else if(state == STATE_ALARM)
                        {
                                if(alarm[location] > 0)
                                {
                                        alarm[location] -= 1;
                                }
                        }
                        break;
                
                case(0xfd):     // 1111 1101 P2.1口对应按键按下 K1 数值加
                        if(state == STATE_SETTING)
                        {
                                switch(location)
                                {
                                        case (0):
                                                if(value[0] < 2)
                                                {
                                                        value[0] += 1;
                                                }
                                                
                                                if(value[0] == 2 && value[1] > 3)
                                                {
                                                        value[1] = 3;
                                                }
                                                
                                                break;
                                        case (1):
                                                if(value[0] < 2 && value[1] < 9)
                                                {
                                                        value[1] += 1;
                                                }
                                                else if(value[0] == 2 && value[1] < 3)
                                                {
                                                        value[1] += 1;
                                                }
                                                break;
                                        case (2):
                                        case (4):
                                                if(value[location] < 5)
                                                {
                                                        value[location] += 1;
                                                }
                                                break;
                                        case (3):
                                        case (5):
                                                if(value[location] < 9)
                                                {
                                                        value[location] += 1;
                                                }
                                                break;
                                }
                        }
                        else if(state == STATE_ALARM)
                        {
                                switch(location)
                                {
                                        case (0):
                                                if(alarm[0] < 2)
                                                {
                                                        alarm[0] += 1;
                                                }
                                                
                                                if(alarm[0] == 2 && alarm[1] > 3)
                                                {
                                                        alarm[1] = 3;
                                                }
                                                
                                                break;
                                        case (1):
                                                if(alarm[0] < 2 && alarm[1] < 9)
                                                {
                                                        alarm[1] += 1;
                                                }
                                                else if(alarm[0] == 2 && alarm[1] < 3)
                                                {
                                                        alarm[1] += 1;
                                                }
                                                break;
                                        case (2):
                                                if(alarm[location] < 5)
                                                {
                                                        alarm[location] += 1;
                                                }
                                                break;
                                        case (3):
                                                if(alarm[location] < 9)
                                                {
                                                        alarm[location] += 1;
                                                }
                                                break;
                                }
                        }
                        break;
                case(0xfb):     // 1111 1011 P2.2口对应按键按下 K2 右移
                        if(state == STATE_SETTING)
                        {
                                location += 1;
                                if(location == 6)
                                {
                                        location = 0;
                                }
                        }
                        else if(state == STATE_ALARM)
                        {
                                location += 1;
                                
                                /* 因为我们设置闹钟的时候只有时和分，所以只有4位 */
                                if(location == 4)
                                {
                                        location = 0;
                                }
                        }
                        break;
                case(0xf7):     // 1111 0111 P2.3口对应按键按下 K3 状态切换
                        switch(state)
                        {
                                case(STATE_RUNNING):
                                        state = STATE_SETTING;
                                        location = 0;
                                        TR0 = 0;
                                        break;
                                case(STATE_SETTING):
                                        state = STATE_ALARM;
                                        location = 0;
                                        TR0 = 0;
                                        break;
                                case(STATE_ALARM):
                                        state = STATE_RUNNING;
                                        TR0 = 1;
                                        break;
                        }
                        break;
        }
}
