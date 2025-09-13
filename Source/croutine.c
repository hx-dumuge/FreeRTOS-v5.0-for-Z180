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
/* 这个协程任务的机制不推荐使用，后期新版本已经废除该文件 */

#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"

/*
 * Some kernel aware debuggers require data to be viewed to be global, rather
 * than file scope.
 */
/*
* 一些内核感知调试器要求将数据视为全局数据，而不是文件范围的数据。
* portREMOVE_STATIC_QUALIFIER 作为static的代替，实现函数调用范围的调试切换
*/
#ifdef portREMOVE_STATIC_QUALIFIER
	#define static
#endif


/* Lists for ready and blocked co-routines. --------------------*/
/* 列出已就绪和已阻塞的协程程序。--------------------*/
/*< 优先就绪协程程序列表 */
static xList pxReadyCoRoutineLists[ configMAX_CO_ROUTINE_PRIORITIES ];	/*< Prioritised ready co-routines. */
/*< 延迟协程程序列表 */
static xList xDelayedCoRoutineList1;									/*< Delayed co-routines. */
/*< 延迟协程程序（使用两个列表 - 一个用于溢出当前滴答计数的延迟。*/
static xList xDelayedCoRoutineList2;									/*< Delayed co-routines (two lists are used - one for delays that have overflowed the current tick count. */
/*< 指向当前正在使用的延迟协程例程列表。 */
static xList * pxDelayedCoRoutineList;									/*< Points to the delayed co-routine list currently being used. */
/*< 指向当前用于保存已溢出当前滴答计数的协程程序的延迟协程程序列表。 */
static xList * pxOverflowDelayedCoRoutineList;							/*< Points to the delayed co-routine list currently being used to hold co-routines that have overflowed the current tick count. */
/*< 保存已由外部事件就绪的协程程序。它们不能直接添加到就绪列表中，因为就绪列表无法通过中断访问。*/
static xList xPendingReadyCoRoutineList;											/*< Holds co-routines that have been readied by an external event.  They cannot be added directly to the ready lists as the ready lists cannot be accessed by interrupts. */

/* Other file private variables. --------------------------------*/
/* 其他文件私有变量。--------------------------------*/
corCRCB * pxCurrentCoRoutine = NULL;
static unsigned portBASE_TYPE uxTopCoRoutineReadyPriority = 0;
/* Tick位数决定类型 */
static portTickType xCoRoutineTickCount = 0, xLastTickCount = 0, xPassedTicks = 0;

/* The initial state of the co-routine when it is created. */
/* 协程例程创建时的初始状态。 */
#define corINITIAL_STATE	( 0 )

/*
 * Place the co-routine represented by pxCRCB into the appropriate ready queue
 * for the priority.  It is inserted at the end of the list.
 *
 * This macro accesses the co-routine ready lists and therefore must not be
 * used from within an ISR.
 */
/*
* 将 pxCRCB 所代表的协程程序放入相应的就绪队列中，并根据优先级将其插入到列表末尾。
*
* 此宏访问协程程序就绪列表，因此不得在中断服务程序 (ISR) 中使用。
*/
#define prvAddCoRoutineToReadyQueue( pxCRCB )																		\
{																													\
	if( pxCRCB->uxPriority > uxTopCoRoutineReadyPriority )															\
	{																												\
		uxTopCoRoutineReadyPriority = pxCRCB->uxPriority;															\
	}																												\
	vListInsertEnd( ( xList * ) &( pxReadyCoRoutineLists[ pxCRCB->uxPriority ] ), &( pxCRCB->xGenericListItem ) );	\
}	

/*
 * Utility to ready all the lists used by the scheduler.  This is called
 * automatically upon the creation of the first co-routine.
 */
/*
* 用于准备调度程序使用的所有列表的实用程序。这将在创建第一个协程例程时自动调用。
*/
static void prvInitialiseCoRoutineLists( void );

/*
 * Co-routines that are readied by an interrupt cannot be placed directly into
 * the ready lists (there is no mutual exclusion).  Instead they are placed in
 * in the pending ready list in order that they can later be moved to the ready
 * list by the co-routine scheduler.
 */
/*
* 由中断就绪的协程例程不能直接放入就绪列表（不存在互斥）。
* 相反它们会被放入待处理就绪列表中，以便稍后由协程例程调度程序将其移至就绪列表。
*/
static void prvCheckPendingReadyList( void );

/*
 * Macro that looks at the list of co-routines that are currently delayed to
 * see if any require waking.
 *
 * Co-routines are stored in the queue in the order of their wake time -
 * meaning once one co-routine has been found whose timer has not expired
 * we need not look any further down the list.
 */
/*
* 该宏用于查看当前延迟的协程程序列表，以查看是否有任何程序需要唤醒。
*
* 协程程序按其唤醒时间的顺序存储在队列中 - 这意味着，一旦找到一个计时器未过期的协程程序，
* 我们无需再继续查找列表。
*/
static void prvCheckDelayedList( void );

/*-----------------------------------------------------------*/

signed portBASE_TYPE xCoRoutineCreate( crCOROUTINE_CODE pxCoRoutineCode, unsigned portBASE_TYPE uxPriority, unsigned portBASE_TYPE uxIndex )
{
signed portBASE_TYPE xReturn;
corCRCB *pxCoRoutine;

	/* Allocate the memory that will store the co-routine control block. */
	/* 分配将存储协程程序控制块的内存。 */
	pxCoRoutine = ( corCRCB * ) pvPortMalloc( sizeof( corCRCB ) );
	if( pxCoRoutine )
	{
		/* If pxCurrentCoRoutine is NULL then this is the first co-routine to
		be created and the co-routine data structures need initialising. */
		/* 如果 pxCurrentCoRoutine 为 NULL，则这是第一个要创建的协程例程，并且协程例程数据结构需要初始化。*/
		if( pxCurrentCoRoutine == NULL )
		{
			pxCurrentCoRoutine = pxCoRoutine;
			prvInitialiseCoRoutineLists();
		}

		/* Check the priority is within limits. */
		/* 检查优先级是否在限制范围内。 */
		if( uxPriority >= configMAX_CO_ROUTINE_PRIORITIES )
		{
			uxPriority = configMAX_CO_ROUTINE_PRIORITIES - 1;
		}

		/* Fill out the co-routine control block from the function parameters. */
		/* 从函数参数中填充协程程序控制块。 */
		pxCoRoutine->uxState = corINITIAL_STATE;
		pxCoRoutine->uxPriority = uxPriority;
		pxCoRoutine->uxIndex = uxIndex;
		pxCoRoutine->pxCoRoutineFunction = pxCoRoutineCode;

		/* Initialise all the other co-routine control block parameters. */
		/* 初始化所有其他协程程序控制块参数。 */
		vListInitialiseItem( &( pxCoRoutine->xGenericListItem ) );
		vListInitialiseItem( &( pxCoRoutine->xEventListItem ) );

		/* Set the co-routine control block as a link back from the xListItem.
		This is so we can get back to the containing CRCB from a generic item
		in a list. */
		/* 将协程程序控制块设置为从 xListItem 返回的链接。这样我们就可以从列表中的通用项返回到包含它的 CRCB。*/
		listSET_LIST_ITEM_OWNER( &( pxCoRoutine->xGenericListItem ), pxCoRoutine );
		listSET_LIST_ITEM_OWNER( &( pxCoRoutine->xEventListItem ), pxCoRoutine );
	
		/* Event lists are always in priority order. */
		/* 事件列表始终按优先级排序。 */
		listSET_LIST_ITEM_VALUE( &( pxCoRoutine->xEventListItem ), configMAX_PRIORITIES - ( portTickType ) uxPriority );
		
		/* Now the co-routine has been initialised it can be added to the ready
		list at the correct priority. */
		/* 现在协程例程已初始化，可以将其添加到正确优先级的就绪列表中。*/
		prvAddCoRoutineToReadyQueue( pxCoRoutine );

		xReturn = pdPASS;
	}
	else
	{		
		xReturn = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
	}
	
	return xReturn;	
}
/*-----------------------------------------------------------*/

void vCoRoutineAddToDelayedList( portTickType xTicksToDelay, xList *pxEventList )
{
portTickType xTimeToWake;

	/* Calculate the time to wake - this may overflow but this is
	not a problem. */
	/* 计算唤醒时间 - 这可能会溢出，但这不是问题。*/
	xTimeToWake = xCoRoutineTickCount + xTicksToDelay;

	/* We must remove ourselves from the ready list before adding
	ourselves to the blocked list as the same list item is used for
	both lists. */
	/* 我们必须先将自己从就绪列表中移除，然后才能将自己添加到阻止列表中，因为两个列表使用相同的列表项。*/
	vListRemove( ( xListItem * ) &( pxCurrentCoRoutine->xGenericListItem ) );

	/* The list item will be inserted in wake time order. */
	/* 列表项将按唤醒时间顺序插入。 */
	listSET_LIST_ITEM_VALUE( &( pxCurrentCoRoutine->xGenericListItem ), xTimeToWake );

	if( xTimeToWake < xCoRoutineTickCount )
	{
		/* Wake time has overflowed.  Place this item in the
		overflow list. */
		/* 唤醒时间已溢出。请将此项目放入溢出列表。*/
		vListInsert( ( xList * ) pxOverflowDelayedCoRoutineList, ( xListItem * ) &( pxCurrentCoRoutine->xGenericListItem ) );
	}
	else
	{
		/* The wake time has not overflowed, so we can use the
		current block list. */
		/* 唤醒时间尚未溢出，因此我们可以使用当前的阻止列表。*/
		vListInsert( ( xList * ) pxDelayedCoRoutineList, ( xListItem * ) &( pxCurrentCoRoutine->xGenericListItem ) );
	}

	if( pxEventList )
	{
		/* Also add the co-routine to an event list.  If this is done then the
		function must be called with interrupts disabled. */
		/* 还要将协程程序添加到事件列表中。如果这样做，则必须在禁用中断的情况下调用该函数。*/
		vListInsert( pxEventList, &( pxCurrentCoRoutine->xEventListItem ) );
	}
}
/*-----------------------------------------------------------*/

static void prvCheckPendingReadyList( void )
{
	/* Are there any co-routines waiting to get moved to the ready list?  These
	are co-routines that have been readied by an ISR.  The ISR cannot access
	the	ready lists itself. */
	/* 是否有任何协程程序正在等待移至就绪列表？这些协程程序已被 ISR 就绪。ISR 本身无法访问就绪列表。*/
	while( !listLIST_IS_EMPTY( &xPendingReadyCoRoutineList ) )
	{
		corCRCB *pxUnblockedCRCB;

		/* The pending ready list can be accessed by an ISR. */
		/* ISR 可以访问挂起就绪列表。*/
		portDISABLE_INTERRUPTS();
		{	
			pxUnblockedCRCB = ( corCRCB * ) listGET_OWNER_OF_HEAD_ENTRY( (&xPendingReadyCoRoutineList) );			
			vListRemove( &( pxUnblockedCRCB->xEventListItem ) );
		}
		portENABLE_INTERRUPTS();

		vListRemove( &( pxUnblockedCRCB->xGenericListItem ) );
		prvAddCoRoutineToReadyQueue( pxUnblockedCRCB );	
	}
}
/*-----------------------------------------------------------*/

static void prvCheckDelayedList( void )
{
corCRCB *pxCRCB;

	xPassedTicks = xTaskGetTickCount() - xLastTickCount;
	while( xPassedTicks )
	{
		xCoRoutineTickCount++;
		xPassedTicks--;

		/* If the tick count has overflowed we need to swap the ready lists. */
		/* 如果滴答计数溢出，我们需要交换就绪列表。*/
		if( xCoRoutineTickCount == 0 )
		{
			xList * pxTemp;

			/* Tick count has overflowed so we need to swap the delay lists.  If there are
			any items in pxDelayedCoRoutineList here then there is an error! */
			/* 时钟周期计数已溢出，因此我们需要交换延迟列表。如果 pxDelayedCoRoutineList 中有任何项目，则会发生错误！*/
			pxTemp = pxDelayedCoRoutineList;
			pxDelayedCoRoutineList = pxOverflowDelayedCoRoutineList;
			pxOverflowDelayedCoRoutineList = pxTemp;
		}

		/* See if this tick has made a timeout expire. */
		/* 看看这个 tick 是否已经超时。 */
		while( ( pxCRCB = ( corCRCB * ) listGET_OWNER_OF_HEAD_ENTRY( pxDelayedCoRoutineList ) ) != NULL )
		{	
			if( xCoRoutineTickCount < listGET_LIST_ITEM_VALUE( &( pxCRCB->xGenericListItem ) ) )				
			{			
				/* Timeout not yet expired. */
				/* 超时尚未到期。 */																		
				break;																				
			}																						

			portDISABLE_INTERRUPTS();
			{
				/* The event could have occurred just before this critical
				section.  If this is the case then the generic list item will
				have been moved to the pending ready list and the following
				line is still valid.  Also the pvContainer parameter will have
				been set to NULL so the following lines are also valid. */
				/* 事件可能发生在此关键代码段之前。如果是这样，则通用列表项将被移至待处理就绪列表，
				* 并且以下行仍然有效。此外，pvContainer 参数将被设置为 NULL，因此以下行也有效。*/
				vListRemove( &( pxCRCB->xGenericListItem ) );											

				/* Is the co-routine waiting on an event also? */
				/* 协程例程是否也在等待事件？*/
				if( pxCRCB->xEventListItem.pvContainer )													
				{															
					vListRemove( &( pxCRCB->xEventListItem ) );											
				}
			}
			portENABLE_INTERRUPTS();

			prvAddCoRoutineToReadyQueue( pxCRCB );													
		}																									
	}

	xLastTickCount = xCoRoutineTickCount;
}
/*-----------------------------------------------------------*/

void vCoRoutineSchedule( void )
{
	/* See if any co-routines readied by events need moving to the ready lists. */
	/* 查看是否有任何由事件准备的协程例程需要移动到就绪列表。 */
	prvCheckPendingReadyList();

	/* See if any delayed co-routines have timed out. */
	/* 查看是否有任何延迟的协程程序已经超时。 */
	prvCheckDelayedList();

	/* Find the highest priority queue that contains ready co-routines. */
	/* 查找包含就绪协程例程的最高优先级队列。 */
	while( listLIST_IS_EMPTY( &( pxReadyCoRoutineLists[ uxTopCoRoutineReadyPriority ] ) ) )
	{
		if( uxTopCoRoutineReadyPriority == 0 )
		{
			/* No more co-routines to check. */
			/* 没有更多协程例程需要检查。 */
			return;
		}
		--uxTopCoRoutineReadyPriority;
	}

	/* listGET_OWNER_OF_NEXT_ENTRY walks through the list, so the co-routines
	 of the	same priority get an equal share of the processor time. */
	 /* listGET_OWNER_OF_NEXT_ENTRY 遍历列表，因此具有相同优先级的协程例程将获得平等的处理器时间份额。*/
	listGET_OWNER_OF_NEXT_ENTRY( pxCurrentCoRoutine, &( pxReadyCoRoutineLists[ uxTopCoRoutineReadyPriority ] ) );

	/* Call the co-routine. */
	( pxCurrentCoRoutine->pxCoRoutineFunction )( pxCurrentCoRoutine, pxCurrentCoRoutine->uxIndex );

	return;
}
/*-----------------------------------------------------------*/

static void prvInitialiseCoRoutineLists( void )
{
unsigned portBASE_TYPE uxPriority;

	for( uxPriority = 0; uxPriority < configMAX_CO_ROUTINE_PRIORITIES; uxPriority++ )
	{
		vListInitialise( ( xList * ) &( pxReadyCoRoutineLists[ uxPriority ] ) );
	}

	vListInitialise( ( xList * ) &xDelayedCoRoutineList1 );
	vListInitialise( ( xList * ) &xDelayedCoRoutineList2 );
	vListInitialise( ( xList * ) &xPendingReadyCoRoutineList );

	/* Start with pxDelayedCoRoutineList using list1 and the
	pxOverflowDelayedCoRoutineList using list2. */
	/* 从使用 list1 的 pxDelayedCoRoutineList 开始，然后使用 list2 的 pxOverflowDelayedCoRoutineList 开始。*/
	pxDelayedCoRoutineList = &xDelayedCoRoutineList1;
	pxOverflowDelayedCoRoutineList = &xDelayedCoRoutineList2;
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xCoRoutineRemoveFromEventList( const xList *pxEventList )
{
corCRCB *pxUnblockedCRCB;
signed portBASE_TYPE xReturn;

	/* This function is called from within an interrupt.  It can only access
	event lists and the pending ready list. */
	/* 此函数在中断中调用。它只能访问事件列表和待处理就绪列表。*/
	pxUnblockedCRCB = ( corCRCB * ) listGET_OWNER_OF_HEAD_ENTRY( pxEventList );
	vListRemove( &( pxUnblockedCRCB->xEventListItem ) );
	vListInsertEnd( ( xList * ) &( xPendingReadyCoRoutineList ), &( pxUnblockedCRCB->xEventListItem ) );

	if( pxUnblockedCRCB->uxPriority >= pxCurrentCoRoutine->uxPriority )
	{
		xReturn = pdTRUE;
	}
	else
	{
		xReturn = pdFALSE;
	}

	return xReturn;
}

