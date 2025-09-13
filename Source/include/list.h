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

/*
 * This is the list implementation used by the scheduler.  While it is tailored
 * heavily for the schedulers needs, it is also available for use by
 * application code.
 *
 * xLists can only store pointers to xListItems.  Each xListItem contains a
 * numeric value (xItemValue).  Most of the time the lists are sorted in
 * descending item value order.
 *
 * Lists are created already containing one list item.  The value of this
 * item is the maximum possible that can be stored, it is therefore always at
 * the end of the list and acts as a marker.  The list member pxHead always
 * points to this marker - even though it is at the tail of the list.  This
 * is because the tail contains a wrap back pointer to the true head of
 * the list.
 *
 * In addition to it's value, each list item contains a pointer to the next
 * item in the list (pxNext), a pointer to the list it is in (pxContainer)
 * and a pointer to back to the object that contains it.  These later two
 * pointers are included for efficiency of list manipulation.  There is
 * effectively a two way link between the object containing the list item and
 * the list item itself.
 *
 *
 * \page ListIntroduction List Implementation
 * \ingroup FreeRTOSIntro
 */

/*
	Changes from V4.3.1

	+ Included local const within listGET_OWNER_OF_NEXT_ENTRY() to assist
	  compiler with optimisation.  Thanks B.R.
*/
/*
	自 V4.3.1 以来的变更

	+ 在 listGET_OWNER_OF_NEXT_ENTRY() 中添加了本地常量，以协助编译器进行优化。感谢 B.R.
*/

/* 这是一个列表的头文件用于队列，里面有链表的定义 */

#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Definition of the only type of object that a list can contain.
 */
/*
* 定义链表可以包含的唯一对象类型。
*/
struct xLIST_ITEM
{
	/*< 列出的值。大多数情况下，这用于按降序对列表进行排序。*/
	portTickType xItemValue;				/*< The value being listed.  In most cases this is used to sort the list in descending order. */
	/*< 指向列表中的下一个 xListItem 的指针。 */
	volatile struct xLIST_ITEM * pxNext;	/*< Pointer to the next xListItem in the list. */
	/*< 指向列表中前一个 xListItem 的指针。 */
	volatile struct xLIST_ITEM * pxPrevious;/*< Pointer to the previous xListItem in the list. */
	/*< 指向包含列表项的对象（通常为 TCB）的指针。因此，包含列表项的对象与列表项本身之间存在双向链接。*/
	void * pvOwner;							/*< Pointer to the object (normally a TCB) that contains the list item.  There is therefore a two way link between the object containing the list item and the list item itself. */
	/*< 指向放置此列表项的列表的指针（如果有）。 */
	void * pvContainer;						/*< Pointer to the list in which this list item is placed (if any). */
};
/* 出于某种原因，两个单独的定义。 */
typedef struct xLIST_ITEM xListItem;		/* For some reason lint wants this as two separate definitions. */

/* 迷你列表项结构 */
struct xMINI_LIST_ITEM
{
	portTickType xItemValue;
	volatile struct xLIST_ITEM *pxNext;
	volatile struct xLIST_ITEM *pxPrevious;
};
typedef struct xMINI_LIST_ITEM xMiniListItem;

/*
 * Definition of the type of queue used by the scheduler.
 */
/*
* 定义调度程序使用的队列类型。
*/
typedef struct xLIST
{
	volatile unsigned portBASE_TYPE uxNumberOfItems;
	/*< 用于遍历列表。指向调用 pvListGetOwnerOfNextEntry () 返回的最后一项。*/
	volatile xListItem * pxIndex;			/*< Used to walk through the list.  Points to the last item returned by a call to pvListGetOwnerOfNextEntry (). */
	/*< 包含最大可能项值的列表项意味着它始终位于列表的末尾，因此用作标记。 */
	volatile xMiniListItem xListEnd;		/*< List item that contains the maximum possible item value meaning it is always at the end of the list and is therefore used as a marker. */
} xList;


/* 下面是一些宏定义的函数 */
/*
* 1. listSET_LIST_ITEM_OWNER( pxListItem, pxOwner )
* 	•	设置链表项的 拥有者。
* 	•	常用：把任务控制块 (TCB) 作为链表项的 owner，这样可以从链表项找到对应任务。
* 2. listSET_LIST_ITEM_VALUE( pxListItem, xValue )
* 	•	设置链表项的排序值。
* 	•	在延时队列里，这个值通常是“唤醒时间”。
* 	•	在优先级队列里，这个值可能是任务优先级。
* 3. listGET_LIST_ITEM_VALUE( pxListItem )
* 	•	获取链表项的排序值。
* 	•	用来判断哪个任务优先被调度，或者哪个超时先到。
* 4. listLIST_IS_EMPTY( pxList )
* 	•	判断链表是否为空。
* 	•	返回 true / false。
* 5. listCURRENT_LIST_LENGTH( pxList )
* 	•	返回链表里当前的项数 (uxNumberOfItems)。
* 6. listGET_OWNER_OF_NEXT_ENTRY( pxTCB, pxList )
* 	•	遍历链表，得到下一个节点的 owner。
* 	•	在调度器里常用：比如轮询就绪任务，拿到下一个任务的控制块。
* 7. listGET_OWNER_OF_HEAD_ENTRY( pxList )
* 	•	获取链表头部第一个节点的 owner。
* 	•	在 就绪任务列表 里，这通常就是“最高优先级任务的 TCB”。
* 8. listIS_CONTAINED_WITHIN( pxList, pxListItem )
* 	•	检查某个 list item 是否属于指定 list。
* 	•	防止重复插入或错误移除。
*/

/*
 * Access macro to set the owner of a list item.  The owner of a list item
 * is the object (usually a TCB) that contains the list item.
 *
 * \page listSET_LIST_ITEM_OWNER listSET_LIST_ITEM_OWNER
 * \ingroup LinkedList
 */
/*
* 访问宏以设置列表项的所有者。列表项的所有者是包含该列表项的对象（通常是 TCB）。
*/
#define listSET_LIST_ITEM_OWNER( pxListItem, pxOwner )		( pxListItem )->pvOwner = ( void * ) pxOwner

/*
 * Access macro to set the value of the list item.  In most cases the value is
 * used to sort the list in descending order.
 *
 * \page listSET_LIST_ITEM_VALUE listSET_LIST_ITEM_VALUE
 * \ingroup LinkedList
 */
/*
 * 访问宏以设置列表项的值。大多数情况下，该值用于按降序对列表进行排序。
 */
#define listSET_LIST_ITEM_VALUE( pxListItem, xValue )		( pxListItem )->xItemValue = xValue

/*
 * Access macro the retrieve the value of the list item.  The value can
 * represent anything - for example a the priority of a task, or the time at
 * which a task should be unblocked.
 *
 * \page listGET_LIST_ITEM_VALUE listGET_LIST_ITEM_VALUE
 * \ingroup LinkedList
 */
/*
* 访问宏以检索列表项的值。该值可以表示任何值 - 例如，任务的优先级，或任务应被解除阻塞的时间。
*/
#define listGET_LIST_ITEM_VALUE( pxListItem )				( ( pxListItem )->xItemValue )

/*
 * Access macro to determine if a list contains any items.  The macro will
 * only have the value true if the list is empty.
 *
 * \page listLIST_IS_EMPTY listLIST_IS_EMPTY
 * \ingroup LinkedList
 */
/*
* 访问宏来判断列表是否包含任何项目。该宏仅在列表为空时才为 true。
*/
#define listLIST_IS_EMPTY( pxList )				( ( pxList )->uxNumberOfItems == ( unsigned portBASE_TYPE ) 0 )

/*
 * Access macro to return the number of items in the list.
 */
/*
* 访问宏以返回列表中项目的数量。
*/
#define listCURRENT_LIST_LENGTH( pxList )		( ( pxList )->uxNumberOfItems )

/*
 * Access function to obtain the owner of the next entry in a list.
 *
 * The list member pxIndex is used to walk through a list.  Calling
 * listGET_OWNER_OF_NEXT_ENTRY increments pxIndex to the next item in the list
 * and returns that entries pxOwner parameter.  Using multiple calls to this
 * function it is therefore possible to move through every item contained in
 * a list.
 *
 * The pxOwner parameter of a list item is a pointer to the object that owns
 * the list item.  In the scheduler this is normally a task control block.
 * The pxOwner parameter effectively creates a two way link between the list
 * item and its owner.
 *
 * @param pxList The list from which the next item owner is to be returned.
 *
 * \page listGET_OWNER_OF_NEXT_ENTRY listGET_OWNER_OF_NEXT_ENTRY
 * \ingroup LinkedList
 */
/*
* 访问函数，用于获取列表中下一个条目的所有者。
*
* 列表成员 pxIndex 用于遍历列表。调用 listGET_OWNER_OF_NEXT_ENTRY 会将 pxIndex 递增到列表中的下一个条目，
* 并返回该条目的 pxOwner 参数。多次调用此函数，就可以遍历列表中包含的每个条目。
*
* 列表项的 pxOwner 参数是一个指向拥有该列表项的对象的指针。
* 在调度程序中，这通常是一个任务控制块。
* pxOwner 参数实际上在列表项及其所有者之间创建了双向链接。
*/
#define listGET_OWNER_OF_NEXT_ENTRY( pxTCB, pxList )									\
{																						\
xList * const pxConstList = pxList;														\
	/* Increment the index to the next item and return the item, ensuring */			\
	/* we don't return the marker used at the end of the list.  */						\
	/* 将索引递增至下一个项并返回该项，确保我们不返回列表末尾使用的标记。*/					\
	( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;						\
	if( ( pxConstList )->pxIndex == ( xListItem * ) &( ( pxConstList )->xListEnd ) )	\
	{																					\
		( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;					\
	}																					\
	pxTCB = ( pxConstList )->pxIndex->pvOwner;											\
}


/*
 * Access function to obtain the owner of the first entry in a list.  Lists
 * are normally sorted in ascending item value order.
 *
 * This function returns the pxOwner member of the first item in the list.
 * The pxOwner parameter of a list item is a pointer to the object that owns
 * the list item.  In the scheduler this is normally a task control block.
 * The pxOwner parameter effectively creates a two way link between the list
 * item and its owner.
 *
 * @param pxList The list from which the owner of the head item is to be
 * returned.
 *
 * \page listGET_OWNER_OF_HEAD_ENTRY listGET_OWNER_OF_HEAD_ENTRY
 * \ingroup LinkedList
 */
/*
* 访问函数，用于获取列表中第一个条目的所有者。列表通常按条目值的升序排序。
*
* 此函数返回列表中第一个条目的 pxOwner 成员。
* 列表项的 pxOwner 参数是指向拥有该列表项的对象的指针。
* 在调度程序中，这通常是一个任务控制块。
* pxOwner 参数有效地在列表项及其所有者之间创建了双向链接。
*/
#define listGET_OWNER_OF_HEAD_ENTRY( pxList )  ( ( pxList->uxNumberOfItems != ( unsigned portBASE_TYPE ) 0 ) ? ( (&( pxList->xListEnd ))->pxNext->pvOwner ) : ( NULL ) )

/*
 * Check to see if a list item is within a list.  The list item maintains a
 * "container" pointer that points to the list it is in.  All this macro does
 * is check to see if the container and the list match.
 *
 * @param pxList The list we want to know if the list item is within.
 * @param pxListItem The list item we want to know if is in the list.
 * @return pdTRUE is the list item is in the list, otherwise pdFALSE.
 * pointer against
 */
/*
* 检查列表项是否在列表中。列表项维护一个“容器”指针，指向其所在的列表。此宏的作用是
* 检查容器和列表是否匹配。
*/
#define listIS_CONTAINED_WITHIN( pxList, pxListItem ) ( ( pxListItem )->pvContainer == ( void * ) pxList )


/* 下面是一些声明的函数 */
/* 
* 1. vListInitialise(xList *pxList)
*	•	功能：初始化一个链表（List_t），让它成为空链表。
*	•	做的事：
*	•	设置 uxNumberOfItems = 0
*	•	初始化哨兵节点 xListEnd（value = 最大值，用作尾哨兵）
*	•	设置 pxIndex 指向哨兵节点
*	•	用途：创建一个新的就绪队列、延时队列、事件队列等。
* 2. vListInitialiseItem(xListItem *pxItem)
*	•	功能：初始化一个链表项（ListItem_t）。
*	•	做的事：
*	•	清空 pxNext、pxPrevious
*	•	pvOwner = NULL
*	•	pvContainer = NULL
*	•	用途：每个任务控制块 (TCB) 或延时对象在加入链表前，都要先初始化它的 ListItem。
* 3. vListInsert(xList *pxList, xListItem *pxNewListItem)
*	•	功能：按 xItemValue 排序插入链表。
*	•	做的事：
*	•	遍历链表，找到第一个 xItemValue 大于新节点的节点
*	•	将新节点插入到它前面
*	•	更新 uxNumberOfItems
*	•	用途：
*	•	延时队列：按唤醒 tick 排序
*	•	优先级队列（在一些调度器实现中）
* 4. vListInsertEnd(xList *pxList, xListItem *pxNewListItem)
*	•	功能：直接插入到链表尾部（哨兵前）。
*	•	用途：
*	•	适合 就绪任务队列 或 FIFO 队列，不需要按 value 排序
*	•	比 vListInsert 更高效
* 5. vListRemove(xListItem *pxItemToRemove)
*	•	功能：从所在链表中删除某个节点。
*	•	做的事：
*	•	修改前后节点的 pxNext / pxPrevious 指针
*	•	更新链表长度 uxNumberOfItems--
*	•	将 pvContainer = NULL
*	•	用途：
*	•	任务从就绪队列被调度出去
*	•	延时队列任务唤醒后从链表删除
*	•	队列 / 信号量对象移除任务 
*/


/*
 * Must be called before a list is used!  This initialises all the members
 * of the list structure and inserts the xListEnd item into the list as a
 * marker to the back of the list.
 *
 * @param pxList Pointer to the list being initialised.
 *
 * \page vListInitialise vListInitialise
 * \ingroup LinkedList
 */
/*
* 必须在使用列表之前调用！这将初始化列表结构的所有成员，并将 xListEnd 项作为标记插入到列表中，作为列表末尾的标记。
*/
void vListInitialise( xList *pxList );

/*
 * Must be called before a list item is used.  This sets the list container to
 * null so the item does not think that it is already contained in a list.
 *
 * @param pxItem Pointer to the list item being initialised.
 *
 * \page vListInitialiseItem vListInitialiseItem
 * \ingroup LinkedList
 */
/*
* 必须在使用列表项之前调用。这会将列表容器设置为 null，这样列表项就不会认为自己已经包含在列表中。
*
* @param pxItem 指向正在初始化的列表项的指针。
*/
void vListInitialiseItem( xListItem *pxItem );

/*
 * Insert a list item into a list.  The item will be inserted into the list in
 * a position determined by its item value (descending item value order).
 *
 * @param pxList The list into which the item is to be inserted.
 *
 * @param pxNewListItem The item to that is to be placed in the list.
 *
 * \page vListInsert vListInsert
 * \ingroup LinkedList
 */
/*
* 将列表项插入列表。该项将插入到列表中，其位置由其值决定（按项值降序排列）。
*
* @param pxList 要插入该项的列表。
*
* @param pxNewListItem 要放入列表中的项。
*/
void vListInsert( xList *pxList, xListItem *pxNewListItem );

/*
 * Insert a list item into a list.  The item will be inserted in a position
 * such that it will be the last item within the list returned by multiple
 * calls to listGET_OWNER_OF_NEXT_ENTRY.
 *
 * The list member pvIndex is used to walk through a list.  Calling
 * listGET_OWNER_OF_NEXT_ENTRY increments pvIndex to the next item in the list.
 * Placing an item in a list using vListInsertEnd effectively places the item
 * in the list position pointed to by pvIndex.  This means that every other
 * item within the list will be returned by listGET_OWNER_OF_NEXT_ENTRY before
 * the pvIndex parameter again points to the item being inserted.
 *
 * @param pxList The list into which the item is to be inserted.
 *
 * @param pxNewListItem The list item to be inserted into the list.
 *
 * \page vListInsertEnd vListInsertEnd
 * \ingroup LinkedList
 */
/*
* 将列表项插入到列表中。该项将被插入到某个位置，
* 以便它是多次调用 listGET_OWNER_OF_NEXT_ENTRY 返回的列表中的最后一个项。
*
* 列表成员 pvIndex 用于遍历列表。调用 listGET_OWNER_OF_NEXT_ENTRY 会将 pvIndex 递增到列表中的下一个项。
* 使用 vListInsertEnd 将项放入列表中，实际上会将该项放置在 pvIndex 指向的列表位置。
* 这意味着，在 pvIndex 参数再次指向要插入的项之前，listGET_OWNER_OF_NEXT_ENTRY 将返回列表中的每个其他项。
* pvIndex 参数再次指向要插入的项。
*
* @param pxList 要插入项的列表。
*
* @param pxNewListItem 要插入到列表中的列表项。
*/
void vListInsertEnd( xList *pxList, xListItem *pxNewListItem );

/*
 * Remove an item from a list.  The list item has a pointer to the list that
 * it is in, so only the list item need be passed into the function.
 *
 * @param vListRemove The item to be removed.  The item will remove itself from
 * the list pointed to by it's pxContainer parameter.
 *
 * \page vListRemove vListRemove
 * \ingroup LinkedList
 */
/*
* 从列表中移除一个项目。该列表项包含一个指向其所在列表的指针，因此只需将列表项传递给函数即可。
*
* @param vListRemove 要移除的项目。该项目将从其 pxContainer 参数指向的列表中移除自身。
*/
void vListRemove( xListItem *pxItemToRemove );

#ifdef __cplusplus
}
#endif

#endif

