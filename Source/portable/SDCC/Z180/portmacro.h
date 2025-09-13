/*
 * FreeRTOS Kernel V10.5.1+
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */


#ifndef PORTMACRO_H
#define PORTMACRO_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given Z80 (Z180, Z80N) hardware and SCCZ80 or SDCC compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR                    char
#define portFLOAT                   float
#define portDOUBLE                  double
#define portLONG                    long
#define portSHORT                   int

#define portSTACK_TYPE	unsigned portSHORT
#define portBASE_TYPE	char


/* 8位单片机应使用16位Ticks */
#if( configUSE_16_BIT_TICKS == 1 )
	typedef unsigned portSHORT portTickType;
	#define portMAX_DELAY ( portTickType ) 0xffff
#else
	typedef unsigned portLONG portTickType;
	#define portMAX_DELAY ( portTickType ) 0xffffffff
#endif



/*-----------------------------------------------------------*/

/* General purpose stringify macros. */

#define string(a) __string(a)
#define __string(a) #a

/*-----------------------------------------------------------*/

/* Architecture specifics. */

#define portSTACK_GROWTH            ( -1 )
#define portTICK_PERIOD_MS          ( ( portTickType ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT          1

/*-----------------------------------------------------------*/

/* Critical section management. */
/* 临界区管理。 */
/* 使用全局资源的时候建议进入临界区 */
/* 需要实现保存当前中断状态，禁用中断 */
#define portENTER_CRITICAL()        \
    do{                             \
        __asm__(                    \
            "push af             \n" \
            "push de             \n" \
            "; 使用ld a,i 可以获取IFF2 \n" \
            "ld a,i             \n" \
            "di                 \n" \
            "push af            \n" \
            "pop de             \n" \
            "ld (_afReg),de     \n" \
            "pop de             \n" \
            "pop af             \n" \
            );                      \
    }while(0)

#define portEXIT_CRITICAL()         \
    do{                             \
        __asm__(                    \
            "push af             \n" \
            "push de             \n" \
            "ld de,(_afReg)      \n" \
            "push de             \n" \
            "pop af             \n" \
            "pop de             \n" \
            "; di    ; unneeded \n" \
            "jp PO,.+4          \n" \
            "ei                 \n" \
            "pop af             \n" \
            );                      \
    }while(0)

#define portDISABLE_INTERRUPTS()    \
    do{                             \
        __asm__(                    \
            "di                 \n" \
            );                      \
    }while(0)

#define portENABLE_INTERRUPTS()     \
    do{                             \
        __asm__(                    \
            "ei                 \n" \
            );                      \
    }while(0)

#define portNOP()                   \
    do{                             \
        __asm__(                    \
            "nop                \n" \
            );                      \
    }while(0)

/*-----------------------------------------------------------*/


extern unsigned portSHORT afReg;

/* Kernel utilities. */
/* 任务切换（让出CPU）的底层函数 */
// extern void vPortYield( void );
// #define portYIELD()                 vPortYield()
void vPortYield( void ) __naked;
#define portYIELD()	vPortYield();

/* 从ISR中让出CPU，这个在v5版本可能不需要 */
extern void vPortYieldFromISR( void );
#define portYIELD_FROM_ISR()        vPortYieldFromISR()
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
/* 定义任务函数原型的宏 */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )


/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* PORTMACRO_H */
