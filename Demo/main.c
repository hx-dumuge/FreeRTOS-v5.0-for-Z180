/* Standard includes. */
#include <stdlib.h>
#include "stdio.h"
#include "stdbool.h"

#include "z180.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define set_bit(x, bit) (x |= (1 << bit))
#define clr_bit(x, bit) (x &= ~(1 << bit))
#define rev_bit(x, bit) (x ^= (1 << bit))
#define get_bit(x, bit) ((x & (1 << bit)) >> bit)

extern void timer_isr(void);

static void taskTest1(void *pvParameters);
static void taskTest2(void *pvParameters);

/*
 * Starts all the other tasks, then starts the scheduler. 
 */
void main( void )
{
	portBASE_TYPE xReturn;

  /* 这里的printf走ASCI的A通道 */
	printf("FreeRTOS demo on Z180\n");

	xReturn = xTaskCreate( taskTest1, "taskTest1", configMINIMAL_STACK_SIZE, NULL, 1, ( xTaskHandle * ) NULL );
	xReturn = xTaskCreate( taskTest2, "taskTest2", configMINIMAL_STACK_SIZE, NULL, 1, ( xTaskHandle * ) NULL );

	// printf("Task created %d\n", xReturn);

	/* Finally kick off the scheduler.  This function should never return. */
	vTaskStartScheduler();

	for( ;; );

	/* Should never reach here as the tasks will now be executing under control
	of the scheduler. */
}
/*-----------------------------------------------------------*/

static void taskTest1(void *pvParameters)
{

	printf("Hello FreeRTOS Task1!\n");
	for(;;)
	{


		printf("TickTask1\n");
    // taskYIELD();
    vTaskDelay(1000);
	}
}

static void taskTest2(void *pvParameters)
{
	printf("Hello FreeRTOS Task2!\n");
  vTaskDelay(500);
	for(;;)
	{
		printf("TickTask2\n");
    // taskYIELD();
    vTaskDelay(1000);
	}	
}



void int1_isr(void)
{

}

void int2_isr(void)
{

}

void prt0_isr(void)
{

}

void prt1_isr(void) __preserves_regs(a,b,c,d,e,h,l,iyh,iyl) __naked
{
  // 调用port里的中断，切换任务用
	timer_isr();
}

void dma0_isr(void)
{
  
}

void dma1_isr(void)
{
  
}

void asci0_isr(void)
{
  
}

void asci1_isr(void)
{
  
}

void csio_isr(void)
{
  
}

int putchar(int c)
{
  __asm
    loopput:
      in0 a, (_STAT0)
      and #0x02
      jr z, loopput
      ld a, e
      OUT0 (_TDR0), a
  __endasm;
  return c;
}

int getchar(void)
{
  __asm
    asci0getc:
      in0 a, (_STAT0)
      bit 7, a
      jr z, empty
      in0 e, (_RDR0)
      ret
    empty:
      ld e, #0x00
      ret
  __endasm;
  return 0;
}