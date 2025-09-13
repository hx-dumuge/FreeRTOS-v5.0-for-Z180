/* Standard includes. */
#include <string.h>

// #include "stdio.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "z180.h"


/*-----------------------------------------------------------*/

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */

typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;

/* 保存af寄存器，其实是为了存中断开关的状态，具体就是IFF2 */
unsigned portSHORT afReg;

/*-----------------------------------------------------------*/

/*
 * Macros to set up, restart (reload), and stop the PRT1 Timer used for
 * the System Tick.
 */
/*
* 用于设置、重启（重新加载）和停止用于系统节拍的 PRT1 定时器的宏。
*/

/* 
* 初始化定时器PRT1 
* 时钟配置一下，使其默认1ms中断一次,没想好要怎么实现自动配置    
*/
#define configSETUP_TIMER_INTERRUPT()                           \
    do{                                                         \
        __asm__(                                                \
            "; 18432000 / 2 / 20 / 1000 - 1 = 460            \n" \
            "ld hl,#0x1cc                                    \n" \
            "out0(_RLDR1L),l                                 \n" \
            "out0(_RLDR1H),h                                 \n" \
            "in0 a,(_TCR)                                    \n" \
            "or #0x22                                        \n" \
            "out0 (_TCR),a                                   \n" \
            );                                                  \
    }while(0)

/* 这个有点奇怪啊 */
#define configRESET_TIMER_INTERRUPT()                           \
    do{                                                         \
        __asm__(                                                \
            "in0 a,(_TCR)                                    \n" \
            "in0 a,(_TMDR1L)                                 \n" \
            );                                                  \
    }while(0)

/* 停止定时器？ */
#define configSTOP_TIMER_INTERRUPT()                            \
    do{                                                         \
        __asm__(                                                \
            "in0 a,(_TCR)                                    \n" \
            "xor #0x22                                      \n" \
            "out0 (_TCR),a                                   \n" \
            );                                                  \
    }while(0)



/*
 * Macros to save all the registers, and save the stack pointer into the TCB.
 *
 * The first thing we do is save the flags then disable interrupts. This is to
 * guard our stack against having a context switch interrupt after we have already
 * pushed the registers onto the stack.
 *
 * The interrupts will have been disabled during the call to portSAVE_CONTEXT()
 * so we need not worry about reading/writing to the stack pointer.
 */

/*
* 该宏用于将当前执行上下文推送到堆栈。
*/
#define portSAVE_CONTEXT()          \
    do{                             \
        __asm__(                    \
            "push af            \n" \
            "ld a,i             \n" \
            "di                 \n" \
            "push af ; iff1:iff2\n" \
            "push bc            \n" \
            "push de            \n" \
            "push hl            \n" \
            "exx                \n" \
            "ex af,af           \n" \
            "push af            \n" \
            "push bc            \n" \
            "push de            \n" \
            "push hl            \n" \
            "push ix            \n" \
            "push iy            \n" \
            "ld hl,#0x00        \n" \
            "add hl,sp          \n" \
            "ld de,(_pxCurrentTCB)  \n"\
            "ex de,hl           \n" \
            "ld (hl),e          \n" \
            "inc hl             \n" \
            "ld (hl),d          \n" \
            );                      \
    }while(0)

/*
* 从堆栈恢复执行上下文的宏。
*/
#define portRESTORE_CONTEXT()       \
    do{                             \
        __asm__(                    \
            "ld hl,(_pxCurrentTCB)  \n" \
            "ld e,(hl)          \n" \
            "inc hl             \n" \
            "ld d,(hl)          \n" \
            "ex de,hl           \n" \
            "ld sp,hl           \n" \
            "pop iy             \n" \
            "pop ix             \n" \
            "pop hl             \n" \
            "pop de             \n" \
            "pop bc             \n" \
            "pop af             \n" \
            "ex af,af           \n" \
            "exx                \n" \
            "pop hl             \n" \
            "pop de             \n" \
            "pop bc             \n" \
            "pop af  ; iff1:iff2\n" \
            "; di    ; unneeded \n" \
            "jp PO,.+4          \n" \
            "ei                 \n" \
            "pop af             \n" \
            "ret                \n" \
            );                      \
    }while(0)
/*
* 该宏用于在ISR下将当前执行上下文推送到堆栈。
* 这里就不把这代码放中断代码位置了，用中断函数调用
*/
#define portSAVE_CONTEXT_IN_ISR()   \
    do{                             \
        __asm__(                    \
            "push af            \n" \
            "ld a,#0x7F          \n" \
            "inc a   ; use PV set PE   \n" \
            "push af ; iff1:iff2\n" \
            "push bc            \n" \
            "push de            \n" \
            "push hl            \n" \
            "exx                \n" \
            "ex af,af           \n" \
            "push af            \n" \
            "push bc            \n" \
            "push de            \n" \
            "push hl            \n" \
            "push ix            \n" \
            "push iy            \n" \
            "ld hl,#0x00            \n" \
            "add hl,sp          \n" \
            "ld de,(_pxCurrentTCB)  \n" \
            "ex de,hl           \n" \
            "ld (hl),e          \n" \
            "inc hl             \n" \
            "ld (hl),d          \n" \
            );                      \
    }while(0)
/*
* 该宏用于在ISR下恢复堆栈中的上下文。
*/
#define portRESTORE_CONTEXT_IN_ISR()\
    do{                             \
        __asm__(                    \
            "ld hl,(_pxCurrentTCB)  \n" \
            "ld e,(hl)          \n" \
            "inc hl             \n" \
            "ld d,(hl)          \n" \
            "ex de,hl           \n" \
            "ld sp,hl           \n" \
            "pop iy             \n" \
            "pop ix             \n" \
            "pop hl             \n" \
            "pop de             \n" \
            "pop bc             \n" \
            "pop af             \n" \
            "ex af,af           \n" \
            "exx                \n" \
            "pop hl             \n" \
            "pop de             \n" \
            "pop bc             \n" \
            "pop af  ; iff1:iff2\n" \
            "; di    ; unneeded \n" \
            "jp PO,.+4      \n" \
            "ei                 \n" \
            "pop af             \n" \
            "reti               \n" \
            );                      \
    }while(0)

    
/*-----------------------------------------------------------*/

/*
 * Perform hardware setup to enable ticks from Timer.
 */
/*
* 执行硬件设置以启用定时器的计时。
*/
static void prvSetupTimerInterrupt( void ) __preserves_regs(iyh,iyl);

/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
/*
* 设置新任务的堆栈，使其准备好接受调度程序的控制。寄存器必须按照端口期望找到它们的顺序放入堆栈。
*/
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
    /* Place the parameter on the stack in the expected location. */
    /* 将参数放置在堆栈上的预期位置。 */
    *pxTopOfStack-- = ( portSTACK_TYPE ) pvParameters;

    /* Place the task return address on stack. Not used */
    /* 将任务返回地址放入堆栈。未使用 */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0;

    /* The start of the task code will be popped off the stack last, so place
    it on first. */
    /* 任务代码的开始部分将最后从堆栈中弹出，因此请将其放在最前面。 */
    *pxTopOfStack-- = ( portSTACK_TYPE ) pxCode;

    /* Now the registers. */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0xAFAF;   /* AF  */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0x0404;   /* IF  */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0xBCBC;   /* BC  */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0xDEDE;   /* DE  */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0xEFEF;   /* HL  */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0xFAFA;   /* AF' */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0xCBCB;   /* BC' */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0xEDED;   /* DE' */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0xFEFE;   /* HL' */
    *pxTopOfStack-- = ( portSTACK_TYPE ) 0xCEFA;   /* IX  */
    *pxTopOfStack   = ( portSTACK_TYPE ) 0xADDE;   /* IY  */

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/

/*
* 设置硬件，以便调度程序能够接管控制。这通常
* 会设置一个滴答中断，并设置定时器以达到正确的滴答频率。
*/
portBASE_TYPE xPortStartScheduler( void ) __preserves_regs(a,b,c,d,e,iyh,iyl) __naked
{
    /* Setup the relevant timer hardware to generate the tick. */
    /* 设置相关的定时器硬件来生成滴答声。 */
    prvSetupTimerInterrupt();

    /* Restore the context of the first task that is going to run. */
    /* 恢复将要运行的第一个任务的上下文。 */
    portRESTORE_CONTEXT();

    /* Should not get here. */
    return pdFALSE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void ) __preserves_regs(b,c,d,e,h,l,iyh,iyl)
{
    /*
     * It is unlikely that the Z80 port will get stopped.
     * If required simply disable the tick interrupt here.
     */
    /*
    * Z80 端口不太可能停止。
    * 如果需要，只需在此处禁用滴答中断即可。
    */
    configSTOP_TIMER_INTERRUPT();
}
/*-----------------------------------------------------------*/

/*
 * Manual context switch.  The first thing we do is save the registers so we
 * can use a naked attribute. This is called by the application, so we don't have
 * to check which bank is loaded.
 */
/*
* 手动上下文切换。我们要做的第一件事是保存寄存器，以便可以使用裸属性。这由应用程序调用，因此我们无需
* 检查哪个存储体已加载。
*/
void vPortYield( void ) __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) __naked
{
    /* 保存上下文寄存器 */
    portSAVE_CONTEXT();
    /* 调用标准调度程序上下文切换函数。 */
    vTaskSwitchContext();
    /* 恢复上下文寄存器 */
    portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

/*
 * Manual context switch callable from ISRs. The first thing we do is save
 * the registers so we can use a naked attribute.
 */
/*
* 可从 ISR 调用的手动上下文切换。我们要做的第一件事是保存寄存器，以便使用裸属性。
*/
void vPortYieldFromISR(void)  __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) __naked
{
    /* 在ISR中保存上下文  */
    portSAVE_CONTEXT_IN_ISR();
    vTaskSwitchContext();
    portRESTORE_CONTEXT_IN_ISR();
}
/*-----------------------------------------------------------*/

/*
 * Initialize Timer (PRT1 for YAZ180, and SCZ180 HBIOS).
 */
/* 初始化定时器PRT1 */
void prvSetupTimerInterrupt( void ) __preserves_regs(iyh,iyl)
{
    configSETUP_TIMER_INTERRUPT();
}
/*-----------------------------------------------------------*/

/* 定时器中断服务程序 */
void timer_isr(void) __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) __naked
{
#if configUSE_PREEMPTION == 1
    /*
     * Tick ISR for preemptive scheduler.  We can use a naked attribute as
     * the context is saved at the start of timer_isr().  The tick
     * count is incremented after the context is saved.
     *
     * Context switch function used by the tick.  This must be identical to
     * vPortYield() from the call to vTaskSwitchContext() onwards.  The only
     * difference from vPortYield() is the tick count is incremented as the
     * call comes from the tick ISR.
     */
    /*
    * 抢占式调度器的 Tick ISR。我们可以使用裸属性，因为
    * 上下文在 timer_isr() 开始时保存。Tick 计数会在上下文保存后递增。
    *
    * Tick 使用的上下文切换函数。从调用 vTaskSwitchContext() 开始，该函数必须与
    * vPortYield() 相同。唯一的区别是，Tick 计数会在调用来自 Tick ISR 时递增。
    */
    portSAVE_CONTEXT_IN_ISR();
    configRESET_TIMER_INTERRUPT();
    /* 递增系统的“tick计数” */
    vTaskIncrementTick();
    vTaskSwitchContext();
    portRESTORE_CONTEXT_IN_ISR();
#else
    /*
     * Tick ISR for the cooperative scheduler.  All this does is increment the
     * tick count.  We don't need to switch context, this can only be done by
     * manual calls to taskYIELD();
     */
    /*
    * 为协作式调度器执行 Tick ISR。这只会增加滴答计数。我们不需要切换上下文，这只能通过
    * 手动调用 taskYIELD() 来完成；
    */
    portSAVE_CONTEXT_IN_ISR();
    configRESET_TIMER_INTERRUPT();
    vTaskIncrementTick();
    portRESTORE_CONTEXT_IN_ISR();
#endif
} // configUSE_PREEMPTION
