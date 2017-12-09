#ifndef __MAIN_H
#define __MAIN_H

/* 类型重定义 */
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned long
        
#define STATE_RUNNING   0
#define STATE_SETTING   1
#define STATE_ALARM     2

void Run(void);

#endif /* __MAIN_H */