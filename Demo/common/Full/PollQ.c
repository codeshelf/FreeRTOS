/*
	FreeRTOS V3.2.4 - Copyright (C) 2003-2005 Richard Barry.

	This file is part of the FreeRTOS distribution.

	FreeRTOS is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS, without being obliged to provide
	the source code for any proprietary components.  See the licensing section 
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

	***************************************************************************
	See http://www.FreeRTOS.org for documentation, latest information, license 
	and contact details.  Please ensure to read the configuration and relevant 
	port sections of the online documentation.
	***************************************************************************
*/


/**
 * This is a very simple queue test.  See the BlockQ. c documentation for a more 
 * comprehensive version.
 *
 * Creates two tasks that communicate over a single queue.  One task acts as a 
 * producer, the other a consumer.  
 *
 * The producer loops for three iteration, posting an incrementing number onto the 
 * queue each cycle.  It then delays for a fixed period before doing exactly the 
 * same again.
 *
 * The consumer loops emptying the queue.  Each item removed from the queue is 
 * checked to ensure it contains the expected value.  When the queue is empty it 
 * blocks for a fixed period, then does the same again.
 *
 * All queue access is performed without blocking.  The consumer completely empties 
 * the queue each time it runs so the producer should never find the queue full.  
 *
 * An error is flagged if the consumer obtains an unexpected value or the producer 
 * find the queue is full.
 *
 * \page PollQC pollQ.c
 * \ingroup DemoFiles
 * <HR>
 */

/*
Changes from V2.0.0

	+ Delay periods are now specified using variables and constants of
	  portTickType rather than unsigned portLONG.
*/

#include <stdlib.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "print.h"

/* Demo program include files. */
#include "PollQ.h"

#define pollqSTACK_SIZE		( ( unsigned portSHORT ) 128 )

/* The task that posts the incrementing number onto the queue. */
static void vPolledQueueProducer( void *pvParameters );

/* The task that empties the queue. */
static void vPolledQueueConsumer( void *pvParameters );

/* Variables that are used to check that the tasks are still running with no errors. */
static volatile portSHORT sPollingConsumerCount = 0, sPollingProducerCount = 0;
/*-----------------------------------------------------------*/

void vStartPolledQueueTasks( unsigned portBASE_TYPE uxPriority )
{
static xQueueHandle xPolledQueue;
const unsigned portBASE_TYPE uxQueueSize = 10;

	/* Create the queue used by the producer and consumer. */
	xPolledQueue = xQueueCreate( uxQueueSize, ( unsigned portBASE_TYPE ) sizeof( unsigned portSHORT ) );

	/* Spawn the producer and consumer. */
	xTaskCreate( vPolledQueueConsumer, "QConsNB", pollqSTACK_SIZE, ( void * ) &xPolledQueue, uxPriority, NULL );
	xTaskCreate( vPolledQueueProducer, "QProdNB", pollqSTACK_SIZE, ( void * ) &xPolledQueue, uxPriority, NULL );
}
/*-----------------------------------------------------------*/

static void vPolledQueueProducer( void *pvParameters )
{
unsigned portSHORT usValue = 0, usLoop;
xQueueHandle *pxQueue;
const portTickType xDelay = ( portTickType ) 200 / portTICK_RATE_MS;
const unsigned portSHORT usNumToProduce = 3;
const portCHAR * const pcTaskStartMsg = "Polled queue producer started.\r\n";
const portCHAR * const pcTaskErrorMsg = "Could not post on polled queue.\r\n";
portSHORT sError = pdFALSE;

	/* Queue a message for printing to say the task has started. */
	vPrintDisplayMessage( &pcTaskStartMsg );

	/* The queue being used is passed in as the parameter. */
	pxQueue = ( xQueueHandle * ) pvParameters;

	for( ;; )
	{		
		for( usLoop = 0; usLoop < usNumToProduce; ++usLoop )
		{
			/* Send an incrementing number on the queue without blocking. */
			if( xQueueSend( *pxQueue, ( void * ) &usValue, ( portTickType ) 0 ) != pdPASS )
			{
				/* We should never find the queue full - this is an error. */
				vPrintDisplayMessage( &pcTaskErrorMsg );
				sError = pdTRUE;
			}
			else
			{
				if( sError == pdFALSE )
				{
					/* If an error has ever been recorded we stop incrementing the 
					check variable. */
					++sPollingProducerCount;
				}

				/* Update the value we are going to post next time around. */
				++usValue;
			}
		}

		/* Wait before we start posting again to ensure the consumer runs and 
		empties the queue. */
		vTaskDelay( xDelay );
	}
}
/*-----------------------------------------------------------*/

static void vPolledQueueConsumer( void *pvParameters )
{
unsigned portSHORT usData, usExpectedValue = 0;
xQueueHandle *pxQueue;
const portTickType xDelay = ( portTickType ) 200 / portTICK_RATE_MS;
const portCHAR * const pcTaskStartMsg = "Polled queue consumer started.\r\n";
const portCHAR * const pcTaskErrorMsg = "Incorrect value received on polled queue.\r\n";
portSHORT sError = pdFALSE;

	/* Queue a message for printing to say the task has started. */
	vPrintDisplayMessage( &pcTaskStartMsg );

	/* The queue being used is passed in as the parameter. */
	pxQueue = ( xQueueHandle * ) pvParameters;

	for( ;; )
	{		
		/* Loop until the queue is empty. */
		while( uxQueueMessagesWaiting( *pxQueue ) )
		{
			if( xQueueReceive( *pxQueue, &usData, ( portTickType ) 0 ) == pdPASS )
			{
				if( usData != usExpectedValue )
				{
					/* This is not what we expected to receive so an error has 
					occurred. */
					vPrintDisplayMessage( &pcTaskErrorMsg );
					sError = pdTRUE;
					/* Catch-up to the value we received so our next expected value 
					should again be correct. */
					usExpectedValue = usData;
				}
				else
				{
					if( sError == pdFALSE )
					{
						/* Only increment the check variable if no errors have 
						occurred. */
						++sPollingConsumerCount;
					}
				}
				++usExpectedValue;
			}
		}

		/* Now the queue is empty we block, allowing the producer to place more 
		items in the queue. */
		vTaskDelay( xDelay );
	}
}
/*-----------------------------------------------------------*/

/* This is called to check that all the created tasks are still running with no errors. */
portBASE_TYPE xArePollingQueuesStillRunning( void )
{
static portSHORT sLastPollingConsumerCount = 0, sLastPollingProducerCount = 0;
portBASE_TYPE xReturn;

	if( ( sLastPollingConsumerCount == sPollingConsumerCount ) ||
		( sLastPollingProducerCount == sPollingProducerCount ) 
	  )
	{
		xReturn = pdFALSE;
	}
	else
	{
		xReturn = pdTRUE;
	}

	sLastPollingConsumerCount = sPollingConsumerCount;
	sLastPollingProducerCount = sPollingProducerCount;

	return xReturn;
}
