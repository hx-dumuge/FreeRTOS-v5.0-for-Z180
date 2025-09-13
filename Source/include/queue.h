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

#ifndef INC_FREERTOS_H
	#error "#include FreeRTOS.h" must appear in source files before "#include queue.h"
#endif




#ifndef QUEUE_H
#define QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif
typedef void * xQueueHandle;

/* For internal use only. */
#define	queueSEND_TO_BACK	( 0 )
#define	queueSEND_TO_FRONT	( 1 )

/* 
*队列基本操作函数声明
* ├─ xQueueCreate                // 创建队列
* ├─ vQueueDelete                // 删除队列
* ├─ uxQueueMessagesWaiting      // 查询队列消息数
*
*队列发送/接收函数声明
* ├─ xQueueGenericSend           // 通用发送（支持前/后插入）
* ├─ xQueueGenericReceive        // 通用接收（支持peek）
* ├─ xQueueSendToFront/Back/Send // 宏，调用GenericSend
* ├─ xQueuePeek / xQueueReceive  // 宏，调用GenericReceive
*
*ISR相关队列操作
* ├─ xQueueGenericSendFromISR    // ISR安全发送
* ├─ xQueueReceiveFromISR        // ISR安全接收
* ├─ xQueueIsQueueEmptyFromISR   // ISR安全查询空
* ├─ xQueueIsQueueFullFromISR    // ISR安全查询满
* ├─ uxQueueMessagesWaitingFromISR // ISR安全查询消息数
*
*Alt API（临界区实现的简化版本）
* ├─ xQueueAltGenericSend / xQueueAltGenericReceive
* ├─ xQueueAltSendToFront / xQueueAltSendToBack / xQueueAltReceive / xQueueAltPeek
*
*协程相关队列操作
* ├─ xQueueCRSendFromISR / xQueueCRReceiveFromISR
* ├─ xQueueCRSend / xQueueCRReceive
*
*信号量/互斥量相关
* ├─ xQueueCreateMutex / xQueueCreateCountingSemaphore
* ├─ xQueueTakeMutexRecursive / xQueueGiveMutexRecursive
*
*队列注册表（调试用）
* └─ vQueueAddToRegistry
*/


/**
 * queue. h
 * <pre>
 xQueueHandle xQueueCreate(
                              unsigned portBASE_TYPE uxQueueLength,
                              unsigned portBASE_TYPE uxItemSize
                          );
 * </pre>
 *
 * Creates a new queue instance.  This allocates the storage required by the
 * new queue and returns a handle for the queue.
 *
 * @param uxQueueLength The maximum number of items that the queue can contain.
 *
 * @param uxItemSize The number of bytes each item in the queue will require.
 * Items are queued by copy, not by reference, so this is the number of bytes
 * that will be copied for each posted item.  Each item on the queue must be
 * the same size.
 *
 * @return If the queue is successfully create then a handle to the newly
 * created queue is returned.  If the queue cannot be created then 0 is
 * returned.
 *
 * Example usage:
   <pre>
 struct AMessage
 {
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
 };

 void vATask( void *pvParameters )
 {
 xQueueHandle xQueue1, xQueue2;

    // Create a queue capable of containing 10 unsigned long values.
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );
    if( xQueue1 == 0 )
    {
        // Queue was not created and must not be used.
    }

    // Create a queue capable of containing 10 pointers to AMessage structures.
    // These should be passed by pointer as they contain a lot of data.
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );
    if( xQueue2 == 0 )
    {
        // Queue was not created and must not be used.
    }

    // ... Rest of task code.
 }
 </pre>
 * \defgroup xQueueCreate xQueueCreate
 * \ingroup QueueManagement
 */
/**
* 队列.h
* <pre>
xQueueHandle xQueueCreate(
    unsigned portBASE_TYPE uxQueueLength,
    unsigned portBASE_TYPE uxItemSize
);
* </pre>
*
* 创建一个新的队列实例。这将分配新队列所需的存储空间，并返回该队列的句柄。
* @param uxQueueLength 队列可容纳的最大项目数。
* @param uxItemSize 队列中每个项目所需的字节数。
* 项目是通过复制而不是引用入队，因此这是每个已发布项目将复制的字节数。队列中的每个项目必须具有相同的大小。
* @return 如果队列创建成功，则返回新创建的队列的句柄。
* 如果无法创建队列，则返回 0。
* 
* 示例用法：
<pre>
struct AMessage
{
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
};

void vATask( void *pvParameters )
{
    xQueueHandle xQueue1, xQueue2;
    // 创建一个可容纳 10 个无符号长整型值的队列。
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );
    if( xQueue1 == 0 )
    {
        // 队列未创建，因此不得使用。
    }
    // 创建一个可容纳 10 个指向 AMessage 结构的指针的队列。
    // 这些指针应通过指针传递，因为它们包含大量数据。
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );
    if( xQueue2 == 0 )
    {
        // 队列未创建，因此不得使用。
    }
    // ... 其余任务代码。
}
</pre>
* \defgroup xQueueCreate xQueueCreate
* \ingroup QueueManagement
*/
xQueueHandle xQueueCreate( unsigned portBASE_TYPE uxQueueLength, unsigned portBASE_TYPE uxItemSize );

/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueSendToToFront(
                                   xQueueHandle xQueue,
                                   const void * pvItemToQueue,
                                   portTickType xTicksToWait
                               );
 * </pre>
 *
 * This is a macro that calls xQueueGenericSend().
 *
 * Post an item to the front of a queue.  The item is queued by copy, not by
 * reference.  This function must not be called from an interrupt service
 * routine.  See xQueueSendFromISR () for an alternative which may be used
 * in an ISR.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvItemToQueue A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param xTicksToWait The maximum amount of time the task should block
 * waiting for space to become available on the queue, should it already
 * be full.  The call will return immediately if this is set to 0.  The
 * time is defined in tick periods so the constant portTICK_RATE_MS
 * should be used to convert to real time if this is required.
 *
 * @return pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
 *
 * Example usage:
   <pre>
 struct AMessage
 {
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
 } xMessage;

 unsigned portLONG ulVar = 10UL;

 void vATask( void *pvParameters )
 {
 xQueueHandle xQueue1, xQueue2;
 struct AMessage *pxMessage;

    // Create a queue capable of containing 10 unsigned long values.
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );

    // Create a queue capable of containing 10 pointers to AMessage structures.
    // These should be passed by pointer as they contain a lot of data.
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );

    // ...

    if( xQueue1 != 0 )
    {
        // Send an unsigned long.  Wait for 10 ticks for space to become
        // available if necessary.
        if( xQueueSendToFront( xQueue1, ( void * ) &ulVar, ( portTickType ) 10 ) != pdPASS )
        {
            // Failed to post the message, even after 10 ticks.
        }
    }

    if( xQueue2 != 0 )
    {
        // Send a pointer to a struct AMessage object.  Don't block if the
        // queue is already full.
        pxMessage = & xMessage;
        xQueueSendToFront( xQueue2, ( void * ) &pxMessage, ( portTickType ) 0 );
    }

	// ... Rest of task code.
 }
 </pre>
 * \defgroup xQueueSend xQueueSend
 * \ingroup QueueManagement
 */
/**
* 队列.h
* <pre>
    portBASE_TYPE xQueueSendToToFront(
    xQueueHandle xQueue,
    const void * pvItemToQueue,
    portTickType xTicksToWait
);
* </pre>
*
* 这是一个调用 xQueueGenericSend() 的宏。
* 将一个项目发送到队列的前端。该项目通过复制而不是引用入队。此函数不能从中断服务程序中调用。
* 请参阅 xQueueSendFromISR() 了解可在中断服务程序中使用的替代方案。
* @param xQueue 指向要发送项目的队列的句柄。
* @param pvItemToQueue 指向要放入队列的项目的指针。
* 队列将容纳的项目的大小在创建队列时已定义，因此将从 pvItemToQueue 复制到队列存储区域中。
* @param xTicksToWait 任务应阻塞的最长时间
* 等待队列空间可用（如果队列已满）。
* 如果设置为 0，调用将立即返回。
* 该时间以 tick 周期定义，因此如果需要，应使用常量 portTICK_RATE_MS
* 将其转换为实际时间。
*
* @return pdTRUE（如果项目已成功提交），否则返回 errQUEUE_FULL。
*
* 示例用法：
<pre>
struct AMessage
{
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
} xMessage;

unsigned portLONG ulVar = 10UL;

void vATask( void *pvParameters )
{
    xQueueHandle xQueue1, xQueue2;
    struct AMessage *pxMessage;
    // 创建一个可容纳 10 个无符号长整型值的队列。
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );
    // 创建一个可容纳 10 个指向 AMessage 结构体的指针的队列。
    // 这些指针应该通过指针传递，因为它们包含大量数据。
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );
    // ...
    if( xQueue1 != 0 )
    {
        // 发送一个无符号长整型值。等待 10 个 tick 后，空间可用。
        // 如果需要，等待可用空间。
        if( xQueueSendToFront( xQueue1, ( void * ) &ulVar, ( portTickType ) 10 ) != pdPASS )
        {
            // 即使过了 10 个 tick，消息发送仍然失败。
        }
    }
    if( xQueue2 != 0 )
    {
        // 发送一个指向 struct AMessage 对象的指针。如果队列已满，则不阻塞。
        // pxMessage = & xMessage;
        xQueueSendToFront( xQueue2, ( void * ) &pxMessage, ( portTickType ) 0 );
    }
    // ... 其余任务代码。
}
</pre>
* \defgroup xQueueSend xQueueSend
* \ingroup QueueManagement
*/
#define xQueueSendToFront( xQueue, pvItemToQueue, xTicksToWait ) xQueueGenericSend( xQueue, pvItemToQueue, xTicksToWait, queueSEND_TO_FRONT )

/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueSendToBack(
                                   xQueueHandle xQueue,
                                   const void * pvItemToQueue,
                                   portTickType xTicksToWait
                               );
 * </pre>
 *
 * This is a macro that calls xQueueGenericSend().
 *
 * Post an item to the back of a queue.  The item is queued by copy, not by
 * reference.  This function must not be called from an interrupt service
 * routine.  See xQueueSendFromISR () for an alternative which may be used
 * in an ISR.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvItemToQueue A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param xTicksToWait The maximum amount of time the task should block
 * waiting for space to become available on the queue, should it already
 * be full.  The call will return immediately if this is set to 0.  The
 * time is defined in tick periods so the constant portTICK_RATE_MS
 * should be used to convert to real time if this is required.
 *
 * @return pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
 *
 * Example usage:
   <pre>
 struct AMessage
 {
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
 } xMessage;

 unsigned portLONG ulVar = 10UL;

 void vATask( void *pvParameters )
 {
 xQueueHandle xQueue1, xQueue2;
 struct AMessage *pxMessage;

    // Create a queue capable of containing 10 unsigned long values.
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );

    // Create a queue capable of containing 10 pointers to AMessage structures.
    // These should be passed by pointer as they contain a lot of data.
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );

    // ...

    if( xQueue1 != 0 )
    {
        // Send an unsigned long.  Wait for 10 ticks for space to become
        // available if necessary.
        if( xQueueSendToBack( xQueue1, ( void * ) &ulVar, ( portTickType ) 10 ) != pdPASS )
        {
            // Failed to post the message, even after 10 ticks.
        }
    }

    if( xQueue2 != 0 )
    {
        // Send a pointer to a struct AMessage object.  Don't block if the
        // queue is already full.
        pxMessage = & xMessage;
        xQueueSendToBack( xQueue2, ( void * ) &pxMessage, ( portTickType ) 0 );
    }

	// ... Rest of task code.
 }
 </pre>
 * \defgroup xQueueSend xQueueSend
 * \ingroup QueueManagement
 */
/**
* 队列.h
* <pre>
portBASE_TYPE xQueueSendToBack(
    xQueueHandle xQueue,
    const void * pvItemToQueue,
    portTickType xTicksToWait
);
* </pre>
*
* 这是一个调用 xQueueGenericSend() 的宏。
* 将一个项目发送到队列的末尾。该项目通过复制而不是引用入队。此函数不能从中断服务程序中调用。
* 请参阅 xQueueSendFromISR() 了解可在中断服务程序中使用的替代方案。
* @param xQueue 指向要发送项目的队列的句柄。
* @param pvItemToQueue 指向要放入队列的项目的指针。
* 队列将容纳的项目的大小在创建队列时已定义，因此将从 pvItemToQueue 复制到队列存储区域中。
* @param xTicksToWait 任务应阻塞的最长时间
* 等待队列空间可用（如果队列已满）。
* 如果设置为 0，调用将立即返回。
* 该时间以 tick 周期定义，因此如果需要，应使用常量 portTICK_RATE_MS 将其转换为实际时间。
* @return pdTRUE（如果项目已成功提交），否则返回 errQUEUE_FULL。
*
* 示例用法：
<pre>
struct AMessage
{
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
} xMessage;

unsigned portLONG ulVar = 10UL;

void vATask( void *pvParameters )
{
    xQueueHandle xQueue1, xQueue2;
    struct AMessage *pxMessage;
    // 创建一个可容纳 10 个无符号长整型值的队列。
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );
    // 创建一个可容纳 10 个指向 AMessage 结构体的指针的队列。
    // 这些结构体包含大量数据，因此应通过指针传递。
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );
    // ...
    if( xQueue1 != 0 )
    {
        // 发送一个无符号长整型值。等待 10 个 tick 后，空间可用。
        // 如果需要，等待可用空间。
        if( xQueueSendToBack( xQueue1, ( void * ) &ulVar, ( portTickType ) 10 ) != pdPASS )
        {
            // 即使过了 10 个 tick，消息发送仍然失败。
        }
    }
    if( xQueue2 != 0 )
    {
        // 发送一个指向 struct AMessage 对象的指针。如果队列已满，则不阻塞。
        // pxMessage = & xMessage;
        xQueueSendToBack( xQueue2, ( void * ) &pxMessage, ( portTickType ) 0 );
    }
    // ... 其余任务代码。
}
</pre>
* \defgroup xQueueSend xQueueSend
* \ingroup QueueManagement
*/
#define xQueueSendToBack( xQueue, pvItemToQueue, xTicksToWait ) xQueueGenericSend( xQueue, pvItemToQueue, xTicksToWait, queueSEND_TO_BACK )

/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueSend(
                              xQueueHandle xQueue,
                              const void * pvItemToQueue,
                              portTickType xTicksToWait
                         );
 * </pre>
 *
 * This is a macro that calls xQueueGenericSend().  It is included for
 * backward compatibility with versions of FreeRTOS.org that did not
 * include the xQueueSendToFront() and xQueueSendToBack() macros.  It is
 * equivalent to xQueueSendToBack().
 *
 * Post an item on a queue.  The item is queued by copy, not by reference.
 * This function must not be called from an interrupt service routine.
 * See xQueueSendFromISR () for an alternative which may be used in an ISR.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvItemToQueue A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param xTicksToWait The maximum amount of time the task should block
 * waiting for space to become available on the queue, should it already
 * be full.  The call will return immediately if this is set to 0.  The
 * time is defined in tick periods so the constant portTICK_RATE_MS
 * should be used to convert to real time if this is required.
 *
 * @return pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
 *
 * Example usage:
   <pre>
 struct AMessage
 {
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
 } xMessage;

 unsigned portLONG ulVar = 10UL;

 void vATask( void *pvParameters )
 {
 xQueueHandle xQueue1, xQueue2;
 struct AMessage *pxMessage;

    // Create a queue capable of containing 10 unsigned long values.
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );

    // Create a queue capable of containing 10 pointers to AMessage structures.
    // These should be passed by pointer as they contain a lot of data.
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );

    // ...

    if( xQueue1 != 0 )
    {
        // Send an unsigned long.  Wait for 10 ticks for space to become
        // available if necessary.
        if( xQueueSend( xQueue1, ( void * ) &ulVar, ( portTickType ) 10 ) != pdPASS )
        {
            // Failed to post the message, even after 10 ticks.
        }
    }

    if( xQueue2 != 0 )
    {
        // Send a pointer to a struct AMessage object.  Don't block if the
        // queue is already full.
        pxMessage = & xMessage;
        xQueueSend( xQueue2, ( void * ) &pxMessage, ( portTickType ) 0 );
    }

	// ... Rest of task code.
 }
 </pre>
 * \defgroup xQueueSend xQueueSend
 * \ingroup QueueManagement
 */
/**
* queue.h
* <pre>
portBASE_TYPE xQueueSend(
    xQueueHandle xQueue,
    const void * pvItemToQueue,
    portTickType xTicksToWait
);
* </pre>
*
* 这是一个调用 xQueueGenericSend() 的宏。包含它是为了与 FreeRTOS.org 
* 未包含 xQueueSendToFront() 和 xQueueSendToBack() 宏的版本向后兼容。它等同于 xQueueSendToBack()。
* 将一个项目发布到队列。该项目通过复制而不是引用进行排队。
* 此函数不能从中断服务例程中调用。
* 有关可在中断服务例程中使用的替代方案，请参阅 xQueueSendFromISR()。
* @param xQueue 要将项目发布到的队列的句柄。
* @param pvItemToQueue 指向要放入队列的项目的指针。队列可容纳的项目大小在队列创建时已定义，
* 因此将从 pvItemToQueue 复制这些字节到队列存储区域。
* @param xTicksToWait 任务应阻塞的最长时间，等待队列空间可用（如果队列已满）。如果设置为 0，
* 调用将立即返回。时间以 tick 周期定义，因此如果需要，应使用常量 portTICK_RATE_MS 将其转换为实时时间。
* @return 如果项目已成功提交，则返回 pdTRUE，否则返回 errQUEUE_FULL。
*
* 示例用法：
<pre>
struct AMessage
{
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
} xMessage;

unsigned portLONG ulVar = 10UL;

void vATask( void *pvParameters )
{
    xQueueHandle xQueue1, xQueue2;
    struct AMessage *pxMessage;
    // 创建一个可容纳 10 个无符号长整型值的队列。
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );
    // 创建一个可容纳 10 个指向 AMessage 结构体的指针的队列。
    // 这些指针应该通过指针传递，因为它们包含大量数据。
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );
    // ...
    if( xQueue1 != 0 )
    {
        // 发送一个无符号长整型值。等待 10 个 tick 以便空间可用（如有必要）。
        if( xQueueSend( xQueue1, ( void * ) &ulVar, ( portTickType ) 10 ) != pdPASS )
        {
            // 即使过了 10 个 tick，消息发送仍然失败。
        }
    }
    if( xQueue2 != 0 )
    {
        // 发送一个指向 struct AMessage 对象的指针。如果队列已满，则不阻塞。
        // pxMessage = & xMessage;
        xQueueSend( xQueue2, ( void * ) &pxMessage, ( portTickType ) 0 );
    }
    // ... 其余任务代码。
}
</pre>
* \defgroup xQueueSend xQueueSend
* \ingroup QueueManagement
*/
#define xQueueSend( xQueue, pvItemToQueue, xTicksToWait ) xQueueGenericSend( xQueue, pvItemToQueue, xTicksToWait, queueSEND_TO_BACK )


/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueGenericSend(
									xQueueHandle xQueue,
									const void * pvItemToQueue,
									portTickType xTicksToWait
									portBASE_TYPE xCopyPosition
								);
 * </pre>
 *
 * It is preferred that the macros xQueueSend(), xQueueSendToFront() and
 * xQueueSendToBack() are used in place of calling this function directly.
 *
 * Post an item on a queue.  The item is queued by copy, not by reference.
 * This function must not be called from an interrupt service routine.
 * See xQueueSendFromISR () for an alternative which may be used in an ISR.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvItemToQueue A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param xTicksToWait The maximum amount of time the task should block
 * waiting for space to become available on the queue, should it already
 * be full.  The call will return immediately if this is set to 0.  The
 * time is defined in tick periods so the constant portTICK_RATE_MS
 * should be used to convert to real time if this is required.
 *
 * @param xCopyPosition Can take the value queueSEND_TO_BACK to place the
 * item at the back of the queue, or queueSEND_TO_FRONT to place the item
 * at the front of the queue (for high priority messages).
 *
 * @return pdTRUE if the item was successfully posted, otherwise errQUEUE_FULL.
 *
 * Example usage:
   <pre>
 struct AMessage
 {
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
 } xMessage;

 unsigned portLONG ulVar = 10UL;

 void vATask( void *pvParameters )
 {
 xQueueHandle xQueue1, xQueue2;
 struct AMessage *pxMessage;

    // Create a queue capable of containing 10 unsigned long values.
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );

    // Create a queue capable of containing 10 pointers to AMessage structures.
    // These should be passed by pointer as they contain a lot of data.
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );

    // ...

    if( xQueue1 != 0 )
    {
        // Send an unsigned long.  Wait for 10 ticks for space to become
        // available if necessary.
        if( xQueueGenericSend( xQueue1, ( void * ) &ulVar, ( portTickType ) 10, queueSEND_TO_BACK ) != pdPASS )
        {
            // Failed to post the message, even after 10 ticks.
        }
    }

    if( xQueue2 != 0 )
    {
        // Send a pointer to a struct AMessage object.  Don't block if the
        // queue is already full.
        pxMessage = & xMessage;
        xQueueGenericSend( xQueue2, ( void * ) &pxMessage, ( portTickType ) 0, queueSEND_TO_BACK );
    }

	// ... Rest of task code.
 }
 </pre>
 * \defgroup xQueueSend xQueueSend
 * \ingroup QueueManagement
 */
/**
* 队列.h
* <pre>
portBASE_TYPE xQueueGenericSend(
    xQueueHandle xQueue,
    const void * pvItemToQueue,
    portTickType xTicksToWait
    portBASE_TYPE xCopyPosition
);
* </pre>
*
* 建议使用宏 xQueueSend()、xQueueSendToFront() 和 xQueueSendToBack() 代替直接调用此函数。
* 将一个项目放入队列。该项目通过复制而不是引用进行排队。
* 此函数不能从中断服务程序中调用。
* 有关可在中断服务程序中使用的替代方案，请参阅 xQueueSendFromISR ()。
* @param xQueue 待放入项目的队列的句柄。
* @param pvItemToQueue 指向待放入队列的项目的指针。
* 队列。队列中项目的大小在创建队列时已定义，因此将从 pvItemToQueue 复制这么多字节到队列存储区域。
* @param xTicksToWait 任务应阻塞的最长时间，等待队列空间可用（如果队列已满）。
* 如果设置为 0，调用将立即返回。
* 时间以滴答周期定义，因此如果需要，应使用常量 portTICK_RATE_MS 将其转换为实时时间。
* @param xCopyPosition 可以取值 queueSEND_TO_BACK 表示将项目置于队列尾部，
* 或取值 queueSEND_TO_FRONT 表示将项目置于队列前部（对于高优先级消息）。
* @return pdTRUE 表示项目已成功提交，否则返回 errQUEUE_FULL。
*
* 示例用法：
<pre>
struct AMessage
{
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
} xMessage;

unsigned portLONG ulVar = 10UL;

void vATask( void *pvParameters )
{
    xQueueHandle xQueue1, xQueue2;
    struct AMessage *pxMessage;
    // 创建一个可容纳 10 个无符号长整型值的队列。
    xQueue1 = xQueueCreate( 10, sizeof( unsigned portLONG ) );
    // 创建一个可容纳 10 个指向 AMessage 结构的指针的队列。
    // 这些指针应该通过指针传递，因为它们包含大量数据。
    xQueue2 = xQueueCreate( 10, sizeof( struct AMessage * ) );
    // ...
    if( xQueue1 != 0 )
    {
        // 发送一个无符号长整型值。等待 10 个 tick，以便空间可用。
        // 如果需要。
        if( xQueueGenericSend( xQueue1, ( void * ) &ulVar, ( portTickType ) 10, queueSEND_TO_BACK ) != pdPASS )
        {
            // 即使过了 10 个 tick，消息发送仍然失败。
        }
    }
    if( xQueue2 != 0 )
    {
        // 发送一个指向 struct AMessage 对象的指针。如果队列已满，则不阻塞。
        // pxMessage = & xMessage;
        xQueueGenericSend( xQueue2, ( void * ) &pxMessage, ( portTickType ) 0, queueSEND_TO_BACK );
    }
    // ... 其余任务代码。
}
</pre>
* \defgroup xQueueSend xQueueSend
* \ingroup QueueManagement
*/
signed portBASE_TYPE xQueueGenericSend( xQueueHandle xQueue, const void * const pvItemToQueue, portTickType xTicksToWait, portBASE_TYPE xCopyPosition );

/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueuePeek(
                             xQueueHandle xQueue,
                             void *pvBuffer,
                             portTickType xTicksToWait
                         );</pre>
 *
 * This is a macro that calls the xQueueGenericReceive() function.
 *
 * Receive an item from a queue without removing the item from the queue.
 * The item is received by copy so a buffer of adequate size must be
 * provided.  The number of bytes copied into the buffer was defined when
 * the queue was created.
 *
 * Successfully received items remain on the queue so will be returned again
 * by the next call, or a call to xQueueReceive().
 *
 * This macro must not be used in an interrupt service routine.
 *
 * @param pxQueue The handle to the queue from which the item is to be
 * received.
 *
 * @param pvBuffer Pointer to the buffer into which the received item will
 * be copied.
 *
 * @param xTicksToWait The maximum amount of time the task should block
 * waiting for an item to receive should the queue be empty at the time
 * of the call.    The time is defined in tick periods so the constant
 * portTICK_RATE_MS should be used to convert to real time if this is required.
 *
 * @return pdTRUE if an item was successfully received from the queue,
 * otherwise pdFALSE.
 *
 * Example usage:
   <pre>
 struct AMessage
 {
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
 } xMessage;

 xQueueHandle xQueue;

 // Task to create a queue and post a value.
 void vATask( void *pvParameters )
 {
 struct AMessage *pxMessage;

    // Create a queue capable of containing 10 pointers to AMessage structures.
    // These should be passed by pointer as they contain a lot of data.
    xQueue = xQueueCreate( 10, sizeof( struct AMessage * ) );
    if( xQueue == 0 )
    {
        // Failed to create the queue.
    }

    // ...

    // Send a pointer to a struct AMessage object.  Don't block if the
    // queue is already full.
    pxMessage = & xMessage;
    xQueueSend( xQueue, ( void * ) &pxMessage, ( portTickType ) 0 );

	// ... Rest of task code.
 }

 // Task to peek the data from the queue.
 void vADifferentTask( void *pvParameters )
 {
 struct AMessage *pxRxedMessage;

    if( xQueue != 0 )
    {
        // Peek a message on the created queue.  Block for 10 ticks if a
        // message is not immediately available.
        if( xQueuePeek( xQueue, &( pxRxedMessage ), ( portTickType ) 10 ) )
        {
            // pcRxedMessage now points to the struct AMessage variable posted
            // by vATask, but the item still remains on the queue.
        }
    }

	// ... Rest of task code.
 }
 </pre>
 * \defgroup xQueueReceive xQueueReceive
 * \ingroup QueueManagement
 */
/**
* 队列.h
* <pre>
portBASE_TYPE xQueuePeek(
    xQueueHandle xQueue,
    void *pvBuffer,
    portTickType xTicksToWait
);</pre>
*
* 这是一个调用 xQueueGenericReceive() 函数的宏。
* 从队列中接收一个项目，但不从队列中移除该项目。
* 由于项目是通过复制接收的，因此必须提供足够大小的缓冲区。复制到缓冲区的字节数是在创建队列时定义的。
* 成功接收的项目将保留在队列中，因此将在下次调用或调用 xQueueReceive() 时再次返回。
* 此宏不能在中断服务程序中使用。
* @param pxQueue 接收项目的队列句柄。
* @param pvBuffer 指向接收项目将被复制到的缓冲区的指针。
* @param xTicksToWait 调用时，如果队列为空，任务应阻塞等待接收数据的最长时间。
* 调用时，队列为空。该时间以滴答周期定义，因此如果需要，应使用常量 portTICK_RATE_MS 将其转换为实时时间。
* @return pdTRUE 如果成功从队列接收数据，
* 否则返回 pdFALSE。
*
* 示例用法：
<pre>
struct AMessage
{
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
} xMessage;

xQueueHandle xQueue;

// 创建队列并发送值的任务。
void vATask( void *pvParameters )
{
    struct AMessage *pxMessage;
    // 创建一个可包含 10 个指向 AMessage 结构体的指针的队列。
    // 这些指针应通过指针传递，因为它们包含大量数据。
    xQueue = xQueueCreate( 10, sizeof( struct AMessage * ) );
    if( xQueue == 0 )
    {
        // 创建队列失败。
    }
    // ...
    // 发送一个指向 struct AMessage 对象的指针。如果队列已满，则阻塞。
    pxMessage = & xMessage;
    xQueueSend( xQueue, ( void * ) &pxMessage, ( portTickType ) 0 );
    // ... 其余任务代码。
}
// 从队列中获取数据的任务。
void vADifferentTask( void *pvParameters )
{
    struct AMessage *pxRxedMessage;
    if( xQueue != 0 )
    {
        // 在已创建的队列中获取一条消息。如果消息不是立即可用的，则阻塞 10 个 tick。
        if( xQueuePeek( xQueue, &( pxRxedMessage ), ( portTickType ) 10 ) )
        {
            // pcRxedMessage 现在指向 vATask 发送的 struct AMessage 变量，
            // 但该消息仍然保留在队列中。
        }
    }
    // ... 其余任务代码。
}
</pre>
* \defgroup xQueueReceive xQueueReceive
* \ingroup QueueManagement
*/
#define xQueuePeek( xQueue, pvBuffer, xTicksToWait ) xQueueGenericReceive( xQueue, pvBuffer, xTicksToWait, pdTRUE )

/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueReceive(
                                 xQueueHandle xQueue,
                                 void *pvBuffer,
                                 portTickType xTicksToWait
                            );</pre>
 *
 * This is a macro that calls the xQueueGenericReceive() function.
 *
 * Receive an item from a queue.  The item is received by copy so a buffer of
 * adequate size must be provided.  The number of bytes copied into the buffer
 * was defined when the queue was created.
 *
 * Successfully received items are removed from the queue.
 *
 * This function must not be used in an interrupt service routine.  See
 * xQueueReceiveFromISR for an alternative that can.
 *
 * @param pxQueue The handle to the queue from which the item is to be
 * received.
 *
 * @param pvBuffer Pointer to the buffer into which the received item will
 * be copied.
 *
 * @param xTicksToWait The maximum amount of time the task should block
 * waiting for an item to receive should the queue be empty at the time
 * of the call.    The time is defined in tick periods so the constant
 * portTICK_RATE_MS should be used to convert to real time if this is required.
 *
 * @return pdTRUE if an item was successfully received from the queue,
 * otherwise pdFALSE.
 *
 * Example usage:
   <pre>
 struct AMessage
 {
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
 } xMessage;

 xQueueHandle xQueue;

 // Task to create a queue and post a value.
 void vATask( void *pvParameters )
 {
 struct AMessage *pxMessage;

    // Create a queue capable of containing 10 pointers to AMessage structures.
    // These should be passed by pointer as they contain a lot of data.
    xQueue = xQueueCreate( 10, sizeof( struct AMessage * ) );
    if( xQueue == 0 )
    {
        // Failed to create the queue.
    }

    // ...

    // Send a pointer to a struct AMessage object.  Don't block if the
    // queue is already full.
    pxMessage = & xMessage;
    xQueueSend( xQueue, ( void * ) &pxMessage, ( portTickType ) 0 );

	// ... Rest of task code.
 }

 // Task to receive from the queue.
 void vADifferentTask( void *pvParameters )
 {
 struct AMessage *pxRxedMessage;

    if( xQueue != 0 )
    {
        // Receive a message on the created queue.  Block for 10 ticks if a
        // message is not immediately available.
        if( xQueueReceive( xQueue, &( pxRxedMessage ), ( portTickType ) 10 ) )
        {
            // pcRxedMessage now points to the struct AMessage variable posted
            // by vATask.
        }
    }

	// ... Rest of task code.
 }
 </pre>
 * \defgroup xQueueReceive xQueueReceive
 * \ingroup QueueManagement
 */
/**
* 队列.h
* <pre>
portBASE_TYPE xQueueReceive(
    xQueueHandle xQueue,
    void *pvBuffer,
    portTickType xTicksToWait
);</pre>
*
* 这是一个调用 xQueueGenericReceive() 函数的宏。
* 从队列接收一个项目。该项目通过复制接收，因此必须提供足够大小的缓冲区。复制到缓冲区的字节数是在创建队列时定义的。
* 成功接收的项目将从队列中移除。
* 此函数不能在中断服务程序中使用。请参阅 xQueueReceiveFromISR 了解替代方案。
* @param pxQueue 接收项目的队列句柄。
* @param pvBuffer 指向接收项目将被复制到的缓冲区的指针。
* @param xTicksToWait 调用时，如果队列为空，任务应阻塞等待接收数据的最长时间。
* 调用时，队列为空。该时间以滴答周期定义，因此如果需要，应使用常量 portTICK_RATE_MS 将其转换为实时时间。
* @return pdTRUE 如果成功从队列接收数据，否则返回 pdFALSE。
*
* 示例用法：
<pre>
struct AMessage
{
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
} xMessage;

xQueueHandle xQueue;

// 创建队列并发送值的任务。
void vATask( void *pvParameters )
{
    struct AMessage *pxMessage;
    // 创建一个可包含 10 个指向 AMessage 结构体的指针的队列。
    // 这些指针应通过指针传递，因为它们包含大量数据。
    xQueue = xQueueCreate( 10, sizeof( struct AMessage * ) );
    if( xQueue == 0 )
    {
        // 创建队列失败。
    }
    // ...
    // 发送一个指向 struct AMessage 对象的指针。如果队列已满，则阻塞。
    // pxMessage = & xMessage;
    xQueueSend( xQueue, ( void * ) &pxMessage, ( portTickType ) 0 );
    // ... 其余任务代码。
}

// 要从队列接收的任务。
void vADifferentTask( void *pvParameters )
{
    struct AMessage *pxRxedMessage;
    if( xQueue != 0 )
    {
        // 在创建的队列上接收消息。如果消息无法立即接收，则阻塞 10 个 tick。
        // if( xQueueReceive( xQueue, &( pxRxedMessage ), ( portTickType ) 10 ) )
        {
            // pcRxedMessage 现在指向 vATask 发送的 struct AMessage 变量。
            // 由 vATask 发送。
        }
    }
    // ... 其余任务代码。
}
</pre>
* \defgroup xQueueReceive xQueueReceive
* \ingroup QueueManagement
*/
#define xQueueReceive( xQueue, pvBuffer, xTicksToWait ) xQueueGenericReceive( xQueue, pvBuffer, xTicksToWait, pdFALSE )


/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueGenericReceive(
                                       xQueueHandle xQueue,
                                       void *pvBuffer,
                                       portTickType xTicksToWait
                                       portBASE_TYPE xJustPeek
                                    );</pre>
 *
 * It is preferred that the macro xQueueReceive() be used rather than calling
 * this function directly.
 *
 * Receive an item from a queue.  The item is received by copy so a buffer of
 * adequate size must be provided.  The number of bytes copied into the buffer
 * was defined when the queue was created.
 *
 * This function must not be used in an interrupt service routine.  See
 * xQueueReceiveFromISR for an alternative that can.
 *
 * @param pxQueue The handle to the queue from which the item is to be
 * received.
 *
 * @param pvBuffer Pointer to the buffer into which the received item will
 * be copied.
 *
 * @param xTicksToWait The maximum amount of time the task should block
 * waiting for an item to receive should the queue be empty at the time
 * of the call.    The time is defined in tick periods so the constant
 * portTICK_RATE_MS should be used to convert to real time if this is required.
 *
 * @param xJustPeek When set to true, the item received from the queue is not
 * actually removed from the queue - meaning a subsequent call to
 * xQueueReceive() will return the same item.  When set to false, the item
 * being received from the queue is also removed from the queue.
 *
 * @return pdTRUE if an item was successfully received from the queue,
 * otherwise pdFALSE.
 *
 * Example usage:
   <pre>
 struct AMessage
 {
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
 } xMessage;

 xQueueHandle xQueue;

 // Task to create a queue and post a value.
 void vATask( void *pvParameters )
 {
 struct AMessage *pxMessage;

    // Create a queue capable of containing 10 pointers to AMessage structures.
    // These should be passed by pointer as they contain a lot of data.
    xQueue = xQueueCreate( 10, sizeof( struct AMessage * ) );
    if( xQueue == 0 )
    {
        // Failed to create the queue.
    }

    // ...

    // Send a pointer to a struct AMessage object.  Don't block if the
    // queue is already full.
    pxMessage = & xMessage;
    xQueueSend( xQueue, ( void * ) &pxMessage, ( portTickType ) 0 );

	// ... Rest of task code.
 }

 // Task to receive from the queue.
 void vADifferentTask( void *pvParameters )
 {
 struct AMessage *pxRxedMessage;

    if( xQueue != 0 )
    {
        // Receive a message on the created queue.  Block for 10 ticks if a
        // message is not immediately available.
        if( xQueueGenericReceive( xQueue, &( pxRxedMessage ), ( portTickType ) 10 ) )
        {
            // pcRxedMessage now points to the struct AMessage variable posted
            // by vATask.
        }
    }

	// ... Rest of task code.
 }
 </pre>
 * \defgroup xQueueReceive xQueueReceive
 * \ingroup QueueManagement
 */
/**
* 队列. h
* <pre>
portBASE_TYPE xQueueGenericReceive(
    xQueueHandle xQueue,
    void *pvBuffer,
    portTickType xTicksToWait
    portBASE_TYPE xJustPeek
);</pre>
*
* 建议使用宏 xQueueReceive()，而不是直接调用此函数。
* 从队列接收一个数据项。该数据项通过复制接收，因此必须提供足够大小的缓冲区。
* 复制到缓冲区的字节数在创建队列时定义。
* 此函数不能在中断服务程序中使用。请参阅 xQueueReceiveFromISR 了解替代方案。
* @param pxQueue 接收数据的队列句柄。
* @param pvBuffer 指向接收数据项将被复制到的缓冲区的指针。
* @param xTicksToWait 调用时队列为空时，任务应阻塞等待接收数据的最长时间。
* 该时间以时钟周期定义，因此如果需要，应使用常量 portTICK_RATE_MS 将其转换为实际时间。
* @param xJustPeek 设置为 true 时，从队列接收的数据实际上不会从队列中移除 - 这意味着后续调用
* xQueueReceive() 将返回同一个数据。设置为 false 时，从队列接收的数据也会从队列中移除。
* @return 如果成功从队列接收数据，则返回 pdTRUE；否则返回 pdFALSE。
*
* 示例用法：
<pre>
struct AMessage
{
    portCHAR ucMessageID;
    portCHAR ucData[ 20 ];
} xMessage;

xQueueHandle xQueue;

// 创建队列并发送值的任务。
void vATask( void *pvParameters )
{
    struct AMessage *pxMessage;
    // 创建一个可容纳 10 个指向 AMessage 结构体的指针的队列。
    // 这些指针应该通过指针传递，因为它们包含大量数据。
    xQueue = xQueueCreate( 10, sizeof( struct AMessage * ) );
    if( xQueue == 0 )
    {
    // 创建队列失败。
    }
    // ...
    // 发送一个指向 struct AMessage 对象的指针。如果队列已满，则不阻塞。
    // 队列已满。
    pxMessage = & xMessage;
    xQueueSend( xQueue, ( void * ) &pxMessage, ( portTickType ) 0 );
    // ... 其余任务代码。
}

// 从队列接收数据的任务。
void vADifferentTask( void *pvParameters )
{
    struct AMessage *pxRxedMessage;
    if( xQueue != 0 )
    {
        // 在创建的队列中接收消息。如果消息不是立即可用的，则阻塞 10 个 tick。
        // 如果消息不是立即可用的，则阻塞 10 个 tick。
        if( xQueueGenericReceive( xQueue, &( pxRxedMessage ), ( portTickType ) 10 ) )
        {
        // pcRxedMessage 现在指向 vATask 发送的 struct AMessage 变量。
        // 由 vATask 发送。
        }
    }
    // ... 其余任务代码。
}
</pre>
* \defgroup xQueueReceive xQueueReceive
* \ingroup QueueManagement
*/
signed portBASE_TYPE xQueueGenericReceive( xQueueHandle xQueue, void * const pvBuffer, portTickType xTicksToWait, portBASE_TYPE xJustPeek );

/**
 * queue. h
 * <pre>unsigned portBASE_TYPE uxQueueMessagesWaiting( const xQueueHandle xQueue );</pre>
 *
 * Return the number of messages stored in a queue.
 *
 * @param xQueue A handle to the queue being queried.
 *
 * @return The number of messages available in the queue.
 *
 * \page uxQueueMessagesWaiting uxQueueMessagesWaiting
 * \ingroup QueueManagement
 */
/**
* queue.h
* <pre>unsigned portBASE_TYPE uxQueueMessagesWaiting( const xQueueHandle xQueue );</pre>
*
* 返回队列中存储的消息数量。
* @param xQueue 正在查询的队列的句柄。
* @return 队列中可用的消息数量。
*
* \page uxQueueMessagesWaiting uxQueueMessagesWaiting
* \ingroup QueueManagement
*/
unsigned portBASE_TYPE uxQueueMessagesWaiting( const xQueueHandle xQueue );

/**
 * queue. h
 * <pre>void vQueueDelete( xQueueHandle xQueue );</pre>
 *
 * Delete a queue - freeing all the memory allocated for storing of items
 * placed on the queue.
 *
 * @param xQueue A handle to the queue to be deleted.
 *
 * \page vQueueDelete vQueueDelete
 * \ingroup QueueManagement
 */
/**
* 队列.h
* <pre>void vQueueDelete( xQueueHandle xQueue );</pre>
*
* 删除队列 - 释放所有用于存储队列中项目的内存。放置在队列中。
*
* @param xQueue 待删除队列的句柄。
*
* \page vQueueDelete vQueueDelete
* \ingroup QueueManagement
*/
void vQueueDelete( xQueueHandle xQueue );

/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueSendToFrontFromISR(
                                         xQueueHandle pxQueue,
                                         const void *pvItemToQueue,
                                         portBASE_TYPE *pxHigherPriorityTaskWoken
                                      );
 </pre>
 *
 * This is a macro that calls xQueueGenericSendFromISR().
 *
 * Post an item to the front of a queue.  It is safe to use this macro from
 * within an interrupt service routine.
 *
 * Items are queued by copy not reference so it is preferable to only
 * queue small items, especially when called from an ISR.  In most cases
 * it would be preferable to store a pointer to the item being queued.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvItemToQueue A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param pxHigherPriorityTaskWoken xQueueSendToFrontFromISR() will set
 * *pxHigherPriorityTaskWoken to pdTRUE if sending to the queue caused a task
 * to unblock, and the unblocked task has a priority higher than the currently
 * running task.  If xQueueSendToFromFromISR() sets this value to pdTRUE then
 * a context switch should be requested before the interrupt is exited.
 *
 * @return pdTRUE if the data was successfully sent to the queue, otherwise
 * errQUEUE_FULL.
 *
 * Example usage for buffered IO (where the ISR can obtain more than one value
 * per call):
   <pre>
 void vBufferISR( void )
 {
 portCHAR cIn;
 portBASE_TYPE xHigherPrioritTaskWoken;

    // We have not woken a task at the start of the ISR.
    xHigherPriorityTaskWoken = pdFALSE;

    // Loop until the buffer is empty.
    do
    {
        // Obtain a byte from the buffer.
        cIn = portINPUT_BYTE( RX_REGISTER_ADDRESS );						

        // Post the byte.  
        xQueueSendToFrontFromISR( xRxQueue, &cIn, &xHigherPriorityTaskWoken );

    } while( portINPUT_BYTE( BUFFER_COUNT ) );

    // Now the buffer is empty we can switch context if necessary.
    if( xHigherPriorityTaskWoken )
    {
        taskYIELD ();
    }
 }
 </pre>
 *
 * \defgroup xQueueSendFromISR xQueueSendFromISR
 * \ingroup QueueManagement
 */
#define xQueueSendToFrontFromISR( pxQueue, pvItemToQueue, pxHigherPriorityTaskWoken ) xQueueGenericSendFromISR( pxQueue, pvItemToQueue, pxHigherPriorityTaskWoken, queueSEND_TO_FRONT )


/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueSendToBackFromISR(
                                         xQueueHandle pxQueue,
                                         const void *pvItemToQueue,
                                         portBASE_TYPE *pxHigherPriorityTaskWoken
                                      );
 </pre>
 *
 * This is a macro that calls xQueueGenericSendFromISR().
 *
 * Post an item to the back of a queue.  It is safe to use this macro from
 * within an interrupt service routine.
 *
 * Items are queued by copy not reference so it is preferable to only
 * queue small items, especially when called from an ISR.  In most cases
 * it would be preferable to store a pointer to the item being queued.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvItemToQueue A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param pxHigherPriorityTaskWoken xQueueSendToBackFromISR() will set
 * *pxHigherPriorityTaskWoken to pdTRUE if sending to the queue caused a task
 * to unblock, and the unblocked task has a priority higher than the currently
 * running task.  If xQueueSendToBackFromISR() sets this value to pdTRUE then
 * a context switch should be requested before the interrupt is exited.
 *
 * @return pdTRUE if the data was successfully sent to the queue, otherwise
 * errQUEUE_FULL.
 *
 * Example usage for buffered IO (where the ISR can obtain more than one value
 * per call):
   <pre>
 void vBufferISR( void )
 {
 portCHAR cIn;
 portBASE_TYPE xHigherPriorityTaskWoken;

    // We have not woken a task at the start of the ISR.
    xHigherPriorityTaskWoken = pdFALSE;

    // Loop until the buffer is empty.
    do
    {
        // Obtain a byte from the buffer.
        cIn = portINPUT_BYTE( RX_REGISTER_ADDRESS );						

        // Post the byte.
        xQueueSendToBackFromISR( xRxQueue, &cIn, &xHigherPriorityTaskWoken );

    } while( portINPUT_BYTE( BUFFER_COUNT ) );

    // Now the buffer is empty we can switch context if necessary.
    if( xHigherPriorityTaskWoken )
    {
        taskYIELD ();
    }
 }
 </pre>
 *
 * \defgroup xQueueSendFromISR xQueueSendFromISR
 * \ingroup QueueManagement
 */
#define xQueueSendToBackFromISR( pxQueue, pvItemToQueue, pxHigherPriorityTaskWoken ) xQueueGenericSendFromISR( pxQueue, pvItemToQueue, pxHigherPriorityTaskWoken, queueSEND_TO_BACK )

/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueSendFromISR(
                                     xQueueHandle pxQueue,
                                     const void *pvItemToQueue,
                                     portBASE_TYPE *pxHigherPriorityTaskWoken
                                );
 </pre>
 *
 * This is a macro that calls xQueueGenericSendFromISR().  It is included
 * for backward compatibility with versions of FreeRTOS.org that did not
 * include the xQueueSendToBackFromISR() and xQueueSendToFrontFromISR()
 * macros.
 *
 * Post an item to the back of a queue.  It is safe to use this function from
 * within an interrupt service routine.
 *
 * Items are queued by copy not reference so it is preferable to only
 * queue small items, especially when called from an ISR.  In most cases
 * it would be preferable to store a pointer to the item being queued.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvItemToQueue A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param pxHigherPriorityTaskWoken xQueueSendFromISR() will set
 * *pxHigherPriorityTaskWoken to pdTRUE if sending to the queue caused a task
 * to unblock, and the unblocked task has a priority higher than the currently
 * running task.  If xQueueSendFromISR() sets this value to pdTRUE then
 * a context switch should be requested before the interrupt is exited.
 *
 * @return pdTRUE if the data was successfully sent to the queue, otherwise
 * errQUEUE_FULL.
 *
 * Example usage for buffered IO (where the ISR can obtain more than one value
 * per call):
   <pre>
 void vBufferISR( void )
 {
 portCHAR cIn;
 portBASE_TYPE xHigherPriorityTaskWoken;

    // We have not woken a task at the start of the ISR.
    xHigherPriorityTaskWoken = pdFALSE;

    // Loop until the buffer is empty.
    do
    {
        // Obtain a byte from the buffer.
        cIn = portINPUT_BYTE( RX_REGISTER_ADDRESS );						

        // Post the byte.  
        xQueueSendFromISR( xRxQueue, &cIn, &xHigherPriorityTaskWoken );

    } while( portINPUT_BYTE( BUFFER_COUNT ) );

    // Now the buffer is empty we can switch context if necessary.
    if( xHigherPriorityTaskWoken )
    {
        // Actual macro used here is port specific.
        taskYIELD_FROM_ISR ();
    }
 }
 </pre>
 *
 * \defgroup xQueueSendFromISR xQueueSendFromISR
 * \ingroup QueueManagement
 */
#define xQueueSendFromISR( pxQueue, pvItemToQueue, pxHigherPriorityTaskWoken ) xQueueGenericSendFromISR( pxQueue, pvItemToQueue, pxHigherPriorityTaskWoken, queueSEND_TO_BACK )

/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueGenericSendFromISR(
                                           xQueueHandle pxQueue,
                                           const void *pvItemToQueue,
                                           portBASE_TYPE *pxHigherPriorityTaskWoken,
										   portBASE_TYPE xCopyPosition
                                       );
 </pre>
 *
 * It is preferred that the macros xQueueSendFromISR(),
 * xQueueSendToFrontFromISR() and xQueueSendToBackFromISR() be used in place
 * of calling this function directly.
 *
 * Post an item on a queue.  It is safe to use this function from within an
 * interrupt service routine.
 *
 * Items are queued by copy not reference so it is preferable to only
 * queue small items, especially when called from an ISR.  In most cases
 * it would be preferable to store a pointer to the item being queued.
 *
 * @param xQueue The handle to the queue on which the item is to be posted.
 *
 * @param pvItemToQueue A pointer to the item that is to be placed on the
 * queue.  The size of the items the queue will hold was defined when the
 * queue was created, so this many bytes will be copied from pvItemToQueue
 * into the queue storage area.
 *
 * @param pxHigherPriorityTaskWoken xQueueGenericSendFromISR() will set
 * *pxHigherPriorityTaskWoken to pdTRUE if sending to the queue caused a task
 * to unblock, and the unblocked task has a priority higher than the currently
 * running task.  If xQueueGenericSendFromISR() sets this value to pdTRUE then
 * a context switch should be requested before the interrupt is exited.
 *
 * @param xCopyPosition Can take the value queueSEND_TO_BACK to place the
 * item at the back of the queue, or queueSEND_TO_FRONT to place the item
 * at the front of the queue (for high priority messages).
 *
 * @return pdTRUE if the data was successfully sent to the queue, otherwise
 * errQUEUE_FULL.
 *
 * Example usage for buffered IO (where the ISR can obtain more than one value
 * per call):
   <pre>
 void vBufferISR( void )
 {
 portCHAR cIn;
 portBASE_TYPE xHigherPriorityTaskWokenByPost;

    // We have not woken a task at the start of the ISR.
    xHigherPriorityTaskWokenByPost = pdFALSE;

    // Loop until the buffer is empty.
    do
    {
        // Obtain a byte from the buffer.
        cIn = portINPUT_BYTE( RX_REGISTER_ADDRESS );						

        // Post each byte.
        xQueueGenericSendFromISR( xRxQueue, &cIn, &xHigherPriorityTaskWokenByPost, queueSEND_TO_BACK );

    } while( portINPUT_BYTE( BUFFER_COUNT ) );

    // Now the buffer is empty we can switch context if necessary.  Note that the
    // name of the yield function required is port specific.
    if( xHigherPriorityTaskWokenByPost )
    {
        taskYIELD_YIELD_FROM_ISR();
    }
 }
 </pre>
 *
 * \defgroup xQueueSendFromISR xQueueSendFromISR
 * \ingroup QueueManagement
 */
signed portBASE_TYPE xQueueGenericSendFromISR( xQueueHandle pxQueue, const void * const pvItemToQueue, signed portBASE_TYPE *pxHigherPriorityTaskWoken, portBASE_TYPE xCopyPosition );

/**
 * queue. h
 * <pre>
 portBASE_TYPE xQueueReceiveFromISR(
                                       xQueueHandle pxQueue,
                                       void *pvBuffer,
                                       portBASE_TYPE *pxTaskWoken
                                   );
 * </pre>
 *
 * Receive an item from a queue.  It is safe to use this function from within an
 * interrupt service routine.
 *
 * @param pxQueue The handle to the queue from which the item is to be
 * received.
 *
 * @param pvBuffer Pointer to the buffer into which the received item will
 * be copied.
 *
 * @param pxTaskWoken A task may be blocked waiting for space to become
 * available on the queue.  If xQueueReceiveFromISR causes such a task to
 * unblock *pxTaskWoken will get set to pdTRUE, otherwise *pxTaskWoken will
 * remain unchanged.
 *
 * @return pdTRUE if an item was successfully received from the queue,
 * otherwise pdFALSE.
 *
 * Example usage:
   <pre>

 xQueueHandle xQueue;

 // Function to create a queue and post some values.
 void vAFunction( void *pvParameters )
 {
 portCHAR cValueToPost;
 const portTickType xBlockTime = ( portTickType )0xff;

    // Create a queue capable of containing 10 characters.
    xQueue = xQueueCreate( 10, sizeof( portCHAR ) );
    if( xQueue == 0 )
    {
        // Failed to create the queue.
    }

    // ...

    // Post some characters that will be used within an ISR.  If the queue
    // is full then this task will block for xBlockTime ticks.
    cValueToPost = 'a';
    xQueueSend( xQueue, ( void * ) &cValueToPost, xBlockTime );
    cValueToPost = 'b';
    xQueueSend( xQueue, ( void * ) &cValueToPost, xBlockTime );

    // ... keep posting characters ... this task may block when the queue
    // becomes full.

    cValueToPost = 'c';
    xQueueSend( xQueue, ( void * ) &cValueToPost, xBlockTime );
 }

 // ISR that outputs all the characters received on the queue.
 void vISR_Routine( void )
 {
 portBASE_TYPE xTaskWokenByReceive = pdFALSE;
 portCHAR cRxedChar;

    while( xQueueReceiveFromISR( xQueue, ( void * ) &cRxedChar, &xTaskWokenByReceive) )
    {
        // A character was received.  Output the character now.
        vOutputCharacter( cRxedChar );

        // If removing the character from the queue woke the task that was
        // posting onto the queue cTaskWokenByReceive will have been set to
        // pdTRUE.  No matter how many times this loop iterates only one
        // task will be woken.
    }

    if( cTaskWokenByPost != ( portCHAR ) pdFALSE;
    {
        taskYIELD ();
    }
 }
 </pre>
 * \defgroup xQueueReceiveFromISR xQueueReceiveFromISR
 * \ingroup QueueManagement
 */
signed portBASE_TYPE xQueueReceiveFromISR( xQueueHandle pxQueue, void * const pvBuffer, signed portBASE_TYPE *pxTaskWoken );

/*
 * Utilities to query queue that are safe to use from an ISR.  These utilities
 * should be used only from witin an ISR, or within a critical section.
 */
signed portBASE_TYPE xQueueIsQueueEmptyFromISR( const xQueueHandle pxQueue );
signed portBASE_TYPE xQueueIsQueueFullFromISR( const xQueueHandle pxQueue );
unsigned portBASE_TYPE uxQueueMessagesWaitingFromISR( const xQueueHandle pxQueue );


/* 
 * xQueueAltGenericSend() is an alternative version of xQueueGenericSend().
 * Likewise xQueueAltGenericReceive() is an alternative version of
 * xQueueGenericReceive().
 *
 * The source code that implements the alternative (Alt) API is much 
 * simpler	because it executes everything from within a critical section.  
 * This is	the approach taken by many other RTOSes, but FreeRTOS.org has the 
 * preferred fully featured API too.  The fully featured API has more 
 * complex	code that takes longer to execute, but makes much less use of 
 * critical sections.  Therefore the alternative API sacrifices interrupt 
 * responsiveness to gain execution speed, whereas the fully featured API
 * sacrifices execution speed to ensure better interrupt responsiveness.
 */
/*
* xQueueAltGenericSend() 是 xQueueGenericSend() 的替代版本。
* 同样，xQueueAltGenericReceive() 也是 xQueueGenericReceive() 的替代版本。
*
* 实现替代 (Alt) API 的源代码要简单得多，因为它在临界区内执行所有操作。
* 许多其他 RTOS 也采用这种方法，但 FreeRTOS.org 也提供更受欢迎的全功能 API。
* 全功能 API 的代码更复杂，执行时间更长，但对临界区的使用更少。
* 因此，替代 API 牺牲了中断响应速度来获得执行速度，而全功能 API 则牺牲了执行速度以确保更好的中断响应速度。
*/
signed portBASE_TYPE xQueueAltGenericSend( xQueueHandle pxQueue, const void * const pvItemToQueue, portTickType xTicksToWait, portBASE_TYPE xCopyPosition );
signed portBASE_TYPE xQueueAltGenericReceive( xQueueHandle pxQueue, void * const pvBuffer, portTickType xTicksToWait, portBASE_TYPE xJustPeeking );
#define xQueueAltSendToFront( xQueue, pvItemToQueue, xTicksToWait ) xQueueAltGenericSend( xQueue, pvItemToQueue, xTicksToWait, queueSEND_TO_FRONT )
#define xQueueAltSendToBack( xQueue, pvItemToQueue, xTicksToWait ) xQueueAltGenericSend( xQueue, pvItemToQueue, xTicksToWait, queueSEND_TO_BACK )
#define xQueueAltReceive( xQueue, pvBuffer, xTicksToWait ) xQueueAltGenericReceive( xQueue, pvBuffer, xTicksToWait, pdFALSE )
#define xQueueAltPeek( xQueue, pvBuffer, xTicksToWait ) xQueueAltGenericReceive( xQueue, pvBuffer, xTicksToWait, pdTRUE )

/*
 * The functions defined above are for passing data to and from tasks.  The
 * functions below are the equivalents for passing data to and from
 * co-routines.
 *
 * These functions are called from the co-routine macro implementation and
 * should not be called directly from application code.  Instead use the macro
 * wrappers defined within croutine.h.
 */
/*
* 上面定义的函数用于与任务之间传递数据。以下函数与与协同例程之间传递数据的功能相同。
* 这些函数由协同例程的宏实现调用，并且不应直接从应用程序代码中调用。请使用 croutine.h 中定义的宏包装器。
*/
signed portBASE_TYPE xQueueCRSendFromISR( xQueueHandle pxQueue, const void *pvItemToQueue, signed portBASE_TYPE xCoRoutinePreviouslyWoken );
signed portBASE_TYPE xQueueCRReceiveFromISR( xQueueHandle pxQueue, void *pvBuffer, signed portBASE_TYPE *pxTaskWoken );
signed portBASE_TYPE xQueueCRSend( xQueueHandle pxQueue, const void *pvItemToQueue, portTickType xTicksToWait );
signed portBASE_TYPE xQueueCRReceive( xQueueHandle pxQueue, void *pvBuffer, portTickType xTicksToWait );

/*
 * For internal use only.  Use xSemaphoreCreateMutex() or
 * xSemaphoreCreateCounting() instead of calling these functions directly.
 */
/*
* 仅供内部使用。请使用 xSemaphoreCreateMutex() 或
* xSemaphoreCreateCounting()，而不是直接调用这些函数。
*/
xQueueHandle xQueueCreateMutex( void );
xQueueHandle xQueueCreateCountingSemaphore( unsigned portBASE_TYPE uxCountValue, unsigned portBASE_TYPE uxInitialCount );

/*
 * For internal use only.  Use xSemaphoreTakeMutexRecursive() or
 * xSemaphoreGiveMutexRecursive() instead of calling these functions directly.
 */
/*
* 仅供内部使用。请使用 xSemaphoreTakeMutexRecursive() 或
* xSemaphoreGiveMutexRecursive()，而不是直接调用这些函数。
*/
portBASE_TYPE xQueueTakeMutexRecursive( xQueueHandle xMutex, portTickType xBlockTime );
portBASE_TYPE xQueueGiveMutexRecursive( xQueueHandle xMutex );

/*
 * The registry is provided as a means for kernel aware debuggers to
 * locate queues, semaphores and mutexes.  Call vQueueAddToRegistry() add
 * a queue, semaphore or mutex handle to the registry if you want the handle 
 * to be available to a kernel aware debugger.  If you are not using a kernel 
 * aware debugger then this function can be ignored.
 *
 * configQUEUE_REGISTRY_SIZE defines the maximum number of handles the
 * registry can hold.  configQUEUE_REGISTRY_SIZE must be greater than 0 
 * within FreeRTOSConfig.h for the registry to be available.  Its value
 * does not effect the number of queues, semaphores and mutexes that can be 
 * created - just the number that the registry can hold.
 *
 * @param xQueue The handle of the queue being added to the registry.  This
 * is the handle returned by a call to xQueueCreate().  Semaphore and mutex
 * handles can also be passed in here.
 *
 * @param pcName The name to be associated with the handle.  This is the
 * name that the kernel aware debugger will display.
 */
/*
* 注册表是为内核感知调试器提供的，用于定位队列、信号量和互斥量。如果您希望句柄
* 可供内核感知调试器使用，请调用 vQueueAddToRegistry() 将队列、信号量或互斥量句柄添加到注册表。
* 如果您不使用内核感知调试器，则可以忽略此函数。
*
* configQUEUE_REGISTRY_SIZE 定义注册表可以容纳的最大句柄数量。
* 在 FreeRTOSConfig.h 文件中，configQUEUE_REGISTRY_SIZE 必须大于 0，注册表才可用。其值
* 不会影响可以创建的队列、信号量和互斥量的数量，仅影响注册表可以容纳的数量。
*
* @param xQueue 正在添加到注册表的队列的句柄。这是调用 xQueueCreate() 返回的句柄。信号量和互斥量
* 句柄也可以在此处传递。
*
* @param pcName 与句柄关联的名称。这是内核感知调试器将显示的名称。
*/
#if configQUEUE_REGISTRY_SIZE > 0
	void vQueueAddToRegistry( xQueueHandle xQueue, signed portCHAR *pcName );
#endif




#ifdef __cplusplus
}
#endif

#endif /* QUEUE_H */

