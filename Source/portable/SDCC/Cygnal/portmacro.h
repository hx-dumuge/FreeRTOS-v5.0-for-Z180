/* 
	FreeRTOS.org V5.0.4 - Copyright (C) 2003-2008 Richard Barry.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS.org is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS.org; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS.org, without being obliged to provide
	the source code for any proprietary components.  See the licensing section 
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

    ***************************************************************************
    ***************************************************************************
    *                                                                         *
    * SAVE TIME AND MONEY!  We can port FreeRTOS.org to your own hardware,    *
    * and even write all or part of your application on your behalf.          *
    * See http://www.OpenRTOS.com for details of the services we provide to   *
    * expedite your project.                                                  *
    *                                                                         *
    ***************************************************************************
    ***************************************************************************

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and 
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety 
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting, 
	licensing and training services.
*/

#ifndef PORTMACRO_H
#define PORTMACRO_H

/* 这里中断号5应该是定时器2 */
#if configUSE_PREEMPTION == 0
	void vTimer2ISR( void ) __interrupt 5;
#else
	void vTimer2ISR( void ) __interrupt 5 __naked;
#endif

/*
 * removed from portmacro.h where it was incorrectly placed  - may not be needed in every program
 */
/*
* 从 portmacro.h 中移除，因为该代码位置错误 - 可能并非每个程序都需要
* 中断号4应该是uart0
*/
// void vSerialISR( void ) __interrupt 4;

/*-----------------------------------------------------------
 * Port specific definitions.  
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */
/*--------------------------------------------------------------
* 特定于端口的定义。
* 此文件中的设置将针对给定的硬件和编译器正确配置 FreeRTOS。
* 这些设置不应更改。
*--------------------------------------------------------------
*/
/* Type definitions. */
/* 类型定义。 */
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		float
#define portLONG		long
#define portSHORT		short
#define portSTACK_TYPE	unsigned portCHAR
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

/* Critical section management. */
/* 临界区管理。 */
/* 使用全局资源的时候建议进入临界区 */
/* 需要实现保存当前中断状态，禁用中断 */
#define portENTER_CRITICAL()		__asm		\
									push	ACC	\
									push	IE	\
									__endasm;	\
									EA = 0;

#define portEXIT_CRITICAL()			__asm			\
									pop		ACC		\
									__endasm;		\
									ACC &= 0x80;	\
									IE |= ACC;		\
									__asm			\
									pop		ACC		\
									__endasm;

#define portDISABLE_INTERRUPTS()	EA = 0;
#define portENABLE_INTERRUPTS()		EA = 1;
/*-----------------------------------------------------------*/	

/* Hardware specifics. */
/* 定义内存字节对齐方式 */
#define portBYTE_ALIGNMENT			1
/* 定义任务栈的增长方向 */
#define portSTACK_GROWTH			( 1 )
/* 系统时钟节拍（tick）周期，单位为毫秒 */
#define portTICK_RATE_MS			( ( unsigned portLONG ) 1000 / configTICK_RATE_HZ )		
/*-----------------------------------------------------------*/	

/* Task utilities. */
/* 任务实用程序。 */
/* 任务切换（让出CPU）的底层函数 */
void vPortYield( void ) __naked;
#define portYIELD()	vPortYield();
/*-----------------------------------------------------------*/	

#define portNOP()				__asm	\
									nop \
								__endasm;

/*-----------------------------------------------------------*/	

/* Task function macros as described on the FreeRTOS.org WEB site. */
/* 任务功能宏如 FreeRTOS.org 网站上所述。 */
/* 定义任务函数原型的宏 */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

#endif /* PORTMACRO_H */


