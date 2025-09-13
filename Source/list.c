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


#include <stdlib.h>
#include "FreeRTOS.h"
#include "list.h"

/*-----------------------------------------------------------
 * PUBLIC LIST API documented in list.h
 *----------------------------------------------------------*/

 /* 
* 1. vListInitialise(xList *pxList)
*	?	功能：初始化一个链表（List_t），让它成为空链表。
*	?	做的事：
*	?	设置 uxNumberOfItems = 0
*	?	初始化哨兵节点 xListEnd（value = 最大值，用作尾哨兵）
*	?	设置 pxIndex 指向哨兵节点
*	?	用途：创建一个新的就绪队列、延时队列、事件队列等。
* 2. vListInitialiseItem(xListItem *pxItem)
*	?	功能：初始化一个链表项（ListItem_t）。
*	?	做的事：
*	?	清空 pxNext、pxPrevious
*	?	pvOwner = NULL
*	?	pvContainer = NULL
*	?	用途：每个任务控制块 (TCB) 或延时对象在加入链表前，都要先初始化它的 ListItem。
* 3. vListInsert(xList *pxList, xListItem *pxNewListItem)
*	?	功能：按 xItemValue 排序插入链表。
*	?	做的事：
*	?	遍历链表，找到第一个 xItemValue 大于新节点的节点
*	?	将新节点插入到它前面
*	?	更新 uxNumberOfItems
*	?	用途：
*	?	延时队列：按唤醒 tick 排序
*	?	优先级队列（在一些调度器实现中）
* 4. vListInsertEnd(xList *pxList, xListItem *pxNewListItem)
*	?	功能：直接插入到链表尾部（哨兵前）。
*	?	用途：
*	?	适合 就绪任务队列 或 FIFO 队列，不需要按 value 排序
*	?	比 vListInsert 更高效
* 5. vListRemove(xListItem *pxItemToRemove)
*	?	功能：从所在链表中删除某个节点。
*	?	做的事：
*	?	修改前后节点的 pxNext / pxPrevious 指针
*	?	更新链表长度 uxNumberOfItems--
*	?	将 pvContainer = NULL
*	?	用途：
*	?	任务从就绪队列被调度出去
*	?	延时队列任务唤醒后从链表删除
*	?	队列 / 信号量对象移除任务 
*/


void vListInitialise( xList *pxList )
{
	/* The list structure contains a list item which is used to mark the
	end of the list.  To initialise the list the list end is inserted
	as the only list entry. */
	/* 列表结构包含一个列表项，用于标记列表的结尾。初始化列表时，列表结尾被插入为唯一的列表条目。*/
	pxList->pxIndex = ( xListItem * ) &( pxList->xListEnd );

	/* The list end value is the highest possible value in the list to
	ensure it remains at the end of the list. */
	/* 列表结束值是列表中可能的最大值，以确保它始终位于列表末尾。*/
	pxList->xListEnd.xItemValue = portMAX_DELAY;

	/* The list end next and previous pointers point to itself so we know
	when the list is empty. */
	/* 列表末尾的 next 和 previous 指针都指向自身，因此我们知道列表何时为空。*/
	pxList->xListEnd.pxNext = ( xListItem * ) &( pxList->xListEnd );
	pxList->xListEnd.pxPrevious = ( xListItem * ) &( pxList->xListEnd );

	pxList->uxNumberOfItems = 0;
}
/*-----------------------------------------------------------*/

void vListInitialiseItem( xListItem *pxItem )
{
	/* Make sure the list item is not recorded as being on a list. */
	/* 确保列表项未被记录在列表中。 */
	pxItem->pvContainer = NULL;
}
/*-----------------------------------------------------------*/

void vListInsertEnd( xList *pxList, xListItem *pxNewListItem )
{
volatile xListItem * pxIndex;

	/* Insert a new list item into pxList, but rather than sort the list,
	makes the new list item the last item to be removed by a call to
	pvListGetOwnerOfNextEntry.  This means it has to be the item pointed to by
	the pxIndex member. */
	/* 将新的列表项插入 pxList，但不对列表进行排序，而是通过调用 pvListGetOwnerOfNextEntry 
	将新列表项作为最后一个要移除的项。这意味着它必须是 pxIndex 成员指向的项。*/
	pxIndex = pxList->pxIndex;

	pxNewListItem->pxNext = pxIndex->pxNext;
	pxNewListItem->pxPrevious = pxList->pxIndex;
	pxIndex->pxNext->pxPrevious = ( volatile xListItem * ) pxNewListItem;
	pxIndex->pxNext = ( volatile xListItem * ) pxNewListItem;
	pxList->pxIndex = ( volatile xListItem * ) pxNewListItem;

	/* Remember which list the item is in. */
	pxNewListItem->pvContainer = ( void * ) pxList;

	( pxList->uxNumberOfItems )++;
}
/*-----------------------------------------------------------*/

void vListInsert( xList *pxList, xListItem *pxNewListItem )
{
volatile xListItem *pxIterator;
portTickType xValueOfInsertion;

	/* Insert the new list item into the list, sorted in ulListItem order. */
	/* 将新的列表项插入到列表中，按 ulListItem 顺序排序。 */
	xValueOfInsertion = pxNewListItem->xItemValue;

	/* If the list already contains a list item with the same item value then
	the new list item should be placed after it.  This ensures that TCB's which
	are stored in ready lists (all of which have the same ulListItem value)
	get an equal share of the CPU.  However, if the xItemValue is the same as 
	the back marker the iteration loop below will not end.  This means we need
	to guard against this by checking the value first and modifying the 
	algorithm slightly if necessary. */
	/* 如果列表中已包含具有相同项值的列表项，则新的列表项应放置在其后。
	这可确保存储在就绪列表中（所有列表项都具有相同 ulListItem 值）的 TCB 获得平等的 CPU 共享。
	但是，如果 xItemValue 与后标记相同，则下面的迭代循环将不会结束。
	这意味着我们需要通过先检查值并在必要时稍微修改算法来防止这种情况。*/
	if( xValueOfInsertion == portMAX_DELAY )
	{
		pxIterator = pxList->xListEnd.pxPrevious;
	}
	else
	{
		for( pxIterator = ( xListItem * ) &( pxList->xListEnd ); pxIterator->pxNext->xItemValue <= xValueOfInsertion; pxIterator = pxIterator->pxNext )
		{
			/* There is nothing to do here, we are just iterating to the
			wanted insertion position. */
		}
	}

	pxNewListItem->pxNext = pxIterator->pxNext;
	pxNewListItem->pxNext->pxPrevious = ( volatile xListItem * ) pxNewListItem;
	pxNewListItem->pxPrevious = pxIterator;
	pxIterator->pxNext = ( volatile xListItem * ) pxNewListItem;

	/* Remember which list the item is in.  This allows fast removal of the
	item later. */
	/* 记住该项目位于哪个列表中。这允许稍后快速删除该项目。*/
	pxNewListItem->pvContainer = ( void * ) pxList;

	( pxList->uxNumberOfItems )++;
}
/*-----------------------------------------------------------*/

void vListRemove( xListItem *pxItemToRemove )
{
xList * pxList;

	pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
	pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;
	
	/* The list item knows which list it is in.  Obtain the list from the list
	item. */
	/* 列表项知道它在哪个列表中。从列表项中获取列表。*/
	pxList = ( xList * ) pxItemToRemove->pvContainer;

	/* Make sure the index is left pointing to a valid item. */
	/* 确保索引指向有效项目。 */
	if( pxList->pxIndex == pxItemToRemove )
	{
		pxList->pxIndex = pxItemToRemove->pxPrevious;
	}

	pxItemToRemove->pvContainer = NULL;
	( pxList->uxNumberOfItems )--;
}
/*-----------------------------------------------------------*/

