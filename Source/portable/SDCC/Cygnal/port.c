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


/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the Cygnal port.
 *----------------------------------------------------------*/

/* Standard includes. */
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Constants required to setup timer 2 to produce the RTOS tick. */
/* 设置定时器 2 产生 RTOS 滴答所需的常量。 */
#define portCLOCK_DIVISOR				( ( unsigned portLONG ) 12 )
#define portMAX_TIMER_VALUE				( ( unsigned portLONG ) 0xffff )
#define portENABLE_TIMER				( ( unsigned portCHAR ) 0x04 )
#define portTIMER_2_INTERRUPT_ENABLE	( ( unsigned portCHAR ) 0x20 )

/* The value used in the IE register when a task first starts. */
/* 任务首次启动时在 IE 寄存器中使用的值。 */
#define portGLOBAL_INTERRUPT_BIT	( ( portSTACK_TYPE ) 0x80 )

/* The value used in the PSW register when a task first starts. */
/* 任务首次启动时在 PSW 寄存器中使用的值。 */
#define portINITIAL_PSW				( ( portSTACK_TYPE ) 0x00 )

/* Macro to clear the timer 2 interrupt flag. */
/* 用于清除定时器 2 中断标志​​的宏。 */
#define portCLEAR_INTERRUPT_FLAG()	TMR2CN &= ~0x80;

/* Used during a context switch to store the size of the stack being copied
to or from XRAM. */
/* 在上下文切换期间用于存储复制到 XRAM 或从 XRAM 复制的堆栈的大小。*/
__data static unsigned portCHAR ucStackBytes;

/* Used during a context switch to point to the next byte in XRAM from/to which
a RAM byte is to be copied. */
/* 在上下文切换期间用于指向 XRAM 中的下一个字节，RAM 字节将从该字节复制/复制到该字节。*/
__xdata static portSTACK_TYPE * __data pxXRAMStack;

/* Used during a context switch to point to the next byte in RAM from/to which
an XRAM byte is to be copied. */
/* 在上下文切换期间用于指向 RAM 中的下一个字节，XRAM 字节将从该字节复制/复制到该字节。*/
__data static portSTACK_TYPE * __data pxRAMStack;

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
/* 我们需要 pxCurrentTCB 变量的地址，但不想知道其类型的任何细节。*/
typedef void tskTCB;
extern volatile tskTCB * volatile pxCurrentTCB;

/*
 * Setup the hardware to generate an interrupt off timer 2 at the required 
 * frequency.
 */
/*
* 设置硬件以所需频率生成中断关闭定时器 2。
*/
static void prvSetupTimerInterrupt( void );

/*-----------------------------------------------------------*/
/*
 * Macro that copies the current stack from internal RAM to XRAM.  This is 
 * required as the 8051 only contains enough internal RAM for a single stack, 
 * but we have a stack for every task.
 */
/*
* 该宏用于将当前堆栈从内部 RAM 复制到 XRAM。这是必需的，因为 8051 的内部 RAM 仅够容纳一个堆栈，
* 但每个任务都有一个堆栈。增强型8051不用担心内部RAM问题。
*/
#define portCOPY_STACK_TO_XRAM()																\
{																								\
	/* pxCurrentTCB points to a TCB which itself points to the location into					\
	which the first	stack byte should be copied.  Set pxXRAMStack to point						\
	to the location into which the first stack byte is to be copied. */							\
	pxXRAMStack = ( __xdata portSTACK_TYPE * ) *( ( __xdata portSTACK_TYPE ** ) pxCurrentTCB );	\
																								\
	/* Set pxRAMStack to point to the first byte to be coped from the stack. */					\
	pxRAMStack = ( __data portSTACK_TYPE * __data ) configSTACK_START;							\
																								\
	/* Calculate the size of the stack we are about to copy from the current					\
	stack pointer value. */																		\
	ucStackBytes = SP - ( configSTACK_START - 1 );												\
																								\
	/* Before starting to copy the stack, store the calculated stack size so					\
	the stack can be restored when the task is resumed. */										\
	*pxXRAMStack = ucStackBytes;																\
																								\
	/* Copy each stack byte in turn.  pxXRAMStack is incremented first as we					\
	have already stored the stack size into XRAM. */											\
	while( ucStackBytes )																		\
	{																							\
		pxXRAMStack++;																			\
		*pxXRAMStack = *pxRAMStack;																\
		pxRAMStack++;																			\
		ucStackBytes--;																			\
	}																							\
}
/*-----------------------------------------------------------*/

/*
 * Macro that copies the stack of the task being resumed from XRAM into 
 * internal RAM.
 */
/*
* 将正在恢复的任务的堆栈从 XRAM 复制到 内部 RAM 的宏。
*/
#define portCOPY_XRAM_TO_STACK()																\
{																								\
	/* Setup the pointers as per portCOPY_STACK_TO_XRAM(), but this time to						\
	copy the data back out of XRAM and into the stack. */										\
	pxXRAMStack = ( __xdata portSTACK_TYPE * ) *( ( __xdata portSTACK_TYPE ** ) pxCurrentTCB );	\
	pxRAMStack = ( __data portSTACK_TYPE * __data ) ( configSTACK_START - 1 );					\
																								\
	/* The first value stored in XRAM was the size of the stack - i.e. the						\
	number of bytes we need to copy back. */													\
	ucStackBytes = pxXRAMStack[ 0 ];															\
																								\
	/* Copy the required number of bytes back into the stack. */								\
	do																							\
	{																							\
		pxXRAMStack++;																			\
		pxRAMStack++;																			\
		*pxRAMStack = *pxXRAMStack;																\
		ucStackBytes--;																			\
	} while( ucStackBytes );																	\
																								\
	/* Restore the stack pointer ready to use the restored stack. */							\
	SP = ( unsigned portCHAR ) pxRAMStack;														\
}
/*-----------------------------------------------------------*/

/*
 * Macro to push the current execution context onto the stack, before the stack 
 * is moved to XRAM. 
 */
/*
* 该宏用于在将堆栈移动到 XRAM 之前，将当前执行上下文推送到堆栈。
*/
#define portSAVE_CONTEXT()																		\
{																								\
	__asm																						\
		/* Push ACC first, as when restoring the context it must be restored					\
		last (it is used to set the IE register). */											\
		push	ACC																				\
		/* Store the IE register then disable interrupts. */									\
		push	IE																				\
		clr		_EA																				\
		push	DPL																				\
		push	DPH																				\
		push	b																				\
		push	ar2																				\
		push	ar3																				\
		push	ar4																				\
		push	ar5																				\
		push	ar6																				\
		push	ar7																				\
		push	ar0																				\
		push	ar1																				\
		push	PSW																				\
	__endasm;																					\
		PSW = 0;																				\
	__asm																						\
		push	_bp																				\
	__endasm;																					\
}
/*-----------------------------------------------------------*/

/*
 * Macro that restores the execution context from the stack.  The execution 
 * context was saved into the stack before the stack was copied into XRAM.
 */
/*
* 从堆栈恢复执行上下文的宏。执行上下文在堆栈复制到 XRAM 之前已保存到堆栈中。
*/
#define portRESTORE_CONTEXT()																	\
{																								\
	__asm																						\
		pop		_bp																				\
		pop		PSW																				\
		pop		ar1																				\
		pop		ar0																				\
		pop		ar7																				\
		pop		ar6																				\
		pop		ar5																				\
		pop		ar4																				\
		pop		ar3																				\
		pop		ar2																				\
		pop		b																				\
		pop		DPH																				\
		pop		DPL																				\
		/* The next byte of the stack is the IE register.  Only the global						\
		enable bit forms part of the task context.  Pop off the IE then set						\
		the global enable bit to match that of the stored IE register. */						\
		pop		ACC																				\
		JB		ACC.7,0098$																		\
		CLR		IE.7																			\
		LJMP	0099$																			\
	0098$:																						\
		SETB	IE.7																			\
	0099$:																						\
		/* Finally pop off the ACC, which was the first register saved. */						\
		pop		ACC																				\
		reti																					\
	__endasm;																					\
}
/*-----------------------------------------------------------*/

/* 
 * See header file for description. 
 */
/* 任务创建时用于初始化任务堆栈的底层函数 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
unsigned portLONG ulAddress;
portSTACK_TYPE *pxStartOfStack;

	/* Leave space to write the size of the stack as the first byte. */
	/* 留出空间将堆栈的大小作为第一个字节写入。 */
	pxStartOfStack = pxTopOfStack;
	pxTopOfStack++;

	/* Place a few bytes of known values on the bottom of the stack. 
	This is just useful for debugging and can be uncommented if required. */
	/* 将几个字节的已知值放在栈底。这仅用于调试，如有需要可以取消注释。	*/
	/*
	*pxTopOfStack = 0x11;
	pxTopOfStack++;
	*pxTopOfStack = 0x22;
	pxTopOfStack++;
	*pxTopOfStack = 0x33;
	pxTopOfStack++;
	*/


	/* Simulate how the stack would look after a call to the scheduler tick ISR. 
	The return address that would have been pushed by the MCU. */
	/* 模拟调用调度程序 tick ISR 后堆栈的样子。MCU 会推送的返回地址。*/
	ulAddress = ( unsigned portLONG ) pxCode;
	*pxTopOfStack = ( portSTACK_TYPE ) ulAddress;
	ulAddress >>= 8;
	pxTopOfStack++;
	*pxTopOfStack = ( portSTACK_TYPE ) ( ulAddress );
	pxTopOfStack++;

	/* Next all the registers will have been pushed by portSAVE_CONTEXT(). */
	/* 接下来所有寄存器都将被 portSAVE_CONTEXT() 推送。*/
	*pxTopOfStack = 0xaa;	/* acc */
	pxTopOfStack++;	

	/* We want tasks to start with interrupts enabled. */
	/* 我们希望任务在启用中断的情况下启动。 */
	*pxTopOfStack = portGLOBAL_INTERRUPT_BIT;
	pxTopOfStack++;

	/* The function parameters will be passed in the DPTR and B register as
	a three byte generic pointer is used. */
	/* 函数参数将作为三字节通用指针传递到 DPTR 和 B 寄存器中。*/
	ulAddress = ( unsigned portLONG ) pvParameters;
	*pxTopOfStack = ( portSTACK_TYPE ) ulAddress;	/* DPL */
	ulAddress >>= 8;
	*pxTopOfStack++;
	*pxTopOfStack = ( portSTACK_TYPE ) ulAddress;	/* DPH */
	ulAddress >>= 8;
	pxTopOfStack++;
	*pxTopOfStack = ( portSTACK_TYPE ) ulAddress;	/* b */
	pxTopOfStack++;

	/* The remaining registers are straight forward. */
	/* 其余寄存器很简单。 */
	*pxTopOfStack = 0x02;	/* R2 */
	pxTopOfStack++;
	*pxTopOfStack = 0x03;	/* R3 */
	pxTopOfStack++;
	*pxTopOfStack = 0x04;	/* R4 */
	pxTopOfStack++;
	*pxTopOfStack = 0x05;	/* R5 */
	pxTopOfStack++;
	*pxTopOfStack = 0x06;	/* R6 */
	pxTopOfStack++;
	*pxTopOfStack = 0x07;	/* R7 */
	pxTopOfStack++;
	*pxTopOfStack = 0x00;	/* R0 */
	pxTopOfStack++;
	*pxTopOfStack = 0x01;	/* R1 */
	pxTopOfStack++;
	*pxTopOfStack = 0x00;	/* PSW */
	pxTopOfStack++;
	*pxTopOfStack = 0xbb;	/* BP */

	/* Dont increment the stack size here as we don't want to include
	the stack size byte as part of the stack size count.
	Finally we place the stack size at the beginning. */
	/* 这里不要增加堆栈大小，因为我们不想将堆栈大小字节包含在堆栈大小计数中。最后，我们将堆栈大小放在开头。*/
	*pxStartOfStack = ( portSTACK_TYPE ) ( pxTopOfStack - pxStartOfStack );

	/* Unlike most ports, we return the start of the stack as this is where the
	size of the stack is stored. */
	/* 与大多数移植不同，我们返回堆栈的起始位置，因为这里存储了堆栈的大小。*/
	return pxStartOfStack;
}
/*-----------------------------------------------------------*/

/* 
 * See header file for description. 
 */
/*
* 设置硬件，以便调度程序能够接管控制。这通常
* 会设置一个滴答中断，并设置定时器以达到正确的滴答频率。
*/
portBASE_TYPE xPortStartScheduler( void )
{
	/* Setup timer 2 to generate the RTOS tick. */
	/* 设置定时器 2 来生成 RTOS 滴答。 */
	prvSetupTimerInterrupt();	

	/* Make sure we start with the expected SFR page.  This line should not
	really be required. */
	/* 确保我们从预期的 SFR 页开始。此行实际上并非必需。*/
	SFRPAGE = 0;

	/* Copy the stack for the first task to execute from XRAM into the stack,
	restore the task context from the new stack, then start running the task. */
	/* 将第一个要执行任务的堆栈从 XRAM 复制到堆栈中，从新堆栈恢复任务上下文，然后开始运行该任务。*/
	portCOPY_XRAM_TO_STACK();
	portRESTORE_CONTEXT();

	/* Should never get here! */
	return pdTRUE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* Not implemented for this port. */
}
/*-----------------------------------------------------------*/

/*
 * Manual context switch.  The first thing we do is save the registers so we
 * can use a naked attribute.
 */
/*
* 手动上下文切换。我们要做的第一件事是保存寄存器，以便可以使用裸属性。
*/
void vPortYield( void ) __naked
{
	/* Save the execution context onto the stack, then copy the entire stack
	to XRAM.  This is necessary as the internal RAM is only large enough to
	hold one stack, and we want one per task. 
	PERFORMANCE COULD BE IMPROVED BY ONLY COPYING TO XRAM IF A TASK SWITCH
	IS REQUIRED. */
	/* 将执行上下文保存到堆栈，然后将整个堆栈复制到 XRAM。这是必要的，
	因为内部 RAM 仅够容纳一个堆栈，而我们希望每个任务一个堆栈。
	仅在需要任务切换时才复制到 XRAM，这样可以提高性能。
	*/
	portSAVE_CONTEXT();
	portCOPY_STACK_TO_XRAM();

	/* Call the standard scheduler context switch function. */
	/* 调用标准调度程序上下文切换函数。 */
	vTaskSwitchContext();

	/* Copy the stack of the task about to execute from XRAM into RAM and
	restore it's context ready to run on exiting. */
	/* 将即将执行的任务的堆栈从 XRAM 复制到 RAM，并恢复其上下文，以便在退出时运行。*/
	portCOPY_XRAM_TO_STACK();
	portRESTORE_CONTEXT();
}


/* 定时器中断服务程序 */
#if configUSE_PREEMPTION == 1
	void vTimer2ISR( void ) __interrupt 5 __naked
	{
		/* Preemptive context switch function triggered by the timer 2 ISR.
		This does the same as vPortYield() (see above) with the addition
		of incrementing the RTOS tick count. */
		/* 由定时器 2 ISR 触发的抢占式上下文切换函数。此函数的功能与 vPortYield()（见上文）相同，但会
		增加 RTOS 的滴答计数。*/

		portSAVE_CONTEXT();
		portCOPY_STACK_TO_XRAM();

		vTaskIncrementTick();
		vTaskSwitchContext();
		
		portCLEAR_INTERRUPT_FLAG();
		portCOPY_XRAM_TO_STACK();
		portRESTORE_CONTEXT();
	}
#else
	void vTimer2ISR( void ) __interrupt 5
	{
		/* When using the cooperative scheduler the timer 2 ISR is only 
		required to increment the RTOS tick count. */

		vTaskIncrementTick();
		portCLEAR_INTERRUPT_FLAG();
	}
#endif
/*-----------------------------------------------------------*/

static void prvSetupTimerInterrupt( void )
{
	unsigned portCHAR ucOriginalSFRPage;

	/* Constants calculated to give the required timer capture values. */
	/* 计算常数以给出所需的定时器捕获值。 */
	const unsigned portLONG ulTicksPerSecond = configCPU_CLOCK_HZ / portCLOCK_DIVISOR;
	const unsigned portLONG ulCaptureTime = ulTicksPerSecond / configTICK_RATE_HZ;
	const unsigned portLONG ulCaptureValue = portMAX_TIMER_VALUE - ulCaptureTime;
	const unsigned portCHAR ucLowCaptureByte = ( unsigned portCHAR ) ( ulCaptureValue & ( unsigned portLONG ) 0xff );
	const unsigned portCHAR ucHighCaptureByte = ( unsigned portCHAR ) ( ulCaptureValue >> ( unsigned portLONG ) 8 );

	/* NOTE:  This uses a timer only present on 8052 architecture. */
	/* 注意：这使用仅存在于 8052 架构上的计时器。 */

	/* Remember the current SFR page so we can restore it at the end of the
	function. */
	/* 记住当前的SFR页面，以便我们在函数结束时恢复它。*/
	ucOriginalSFRPage = SFRPAGE;
	SFRPAGE = 0;

	/* TMR2CF can be left in its default state. */	
	/* TMR2CF 可以保留其默认状态。 */
	TMR2CF = ( unsigned portCHAR ) 0;

	/* Setup the overflow reload value. */
	/* 设置溢出重载值。 */
	RCAP2L = ucLowCaptureByte;
	RCAP2H = ucHighCaptureByte;

	/* The initial load is performed manually. */
	/* 初始加载是手动执行的。 */
	TMR2L = ucLowCaptureByte;
	TMR2H = ucHighCaptureByte;

	/* Enable the timer 2 interrupts. */
	IE |= portTIMER_2_INTERRUPT_ENABLE;

	/* Interrupts are disabled when this is called so the timer can be started
	here. */
	TMR2CN = portENABLE_TIMER;

	/* Restore the original SFR page. */
	SFRPAGE = ucOriginalSFRPage;
}




