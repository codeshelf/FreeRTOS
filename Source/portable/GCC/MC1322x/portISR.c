/*
	FreeRTOS.org V5.1.1 - Copyright (C) 2003-2008 Richard Barry.

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
 * Components that can be compiled to either ARM or THUMB mode are
 * contained in port.c  The ISR routines, which can only be compiled
 * to ARM mode, are contained in this file.
 *----------------------------------------------------------*/

/*
	Changes from V2.5.2

	+ The critical section management functions have been changed.  These no
	  longer modify the stack and are safe to use at all optimisation levels.
	  The functions are now also the same for both ARM and THUMB modes.

	Changes from V2.6.0

	+ Removed the 'static' from the definition of vNonPreemptiveTick() to
	  allow the demo to link when using the cooperative scheduler.

	Changes from V3.2.4

	+ The assembler statements are now included in a single asm block rather
	  than each line having its own asm block.
*/


/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Constants required to handle interrupts. */
#define portTIMER_MATCH_ISR_BIT		( ( unsigned portCHAR ) 0x01 )
#define portCLEAR_VIC_INTERRUPT		( ( unsigned portLONG ) 0 )

/* Constants required to handle critical sections. */
#define portNO_CRITICAL_NESTING		( ( unsigned portLONG ) 0 )
volatile unsigned portLONG ulCriticalNesting = 9999UL;

/*-----------------------------------------------------------*/

/* ISR to handle manual context switches (from a call to taskYIELD()). */
void vPortYieldProcessor( void ) __attribute__((interrupt("SWI"), naked));

/*
 * The scheduler can only be started from ARM mode, hence the inclusion of this
 * function here.
 */
void vPortISRStartFirstTask( void );
/*-----------------------------------------------------------*/

void vPortISRStartFirstTask( void )
{
	/* Simply start the scheduler.  This is included here as it can only be
	called from ARM mode. */
	portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

/*
 * Called by portYIELD() or taskYIELD() to manually force a context switch.
 *
 * When a context switch is performed from the task level the saved task
 * context is made to look as if it occurred from within the tick ISR.  This
 * way the same restore context function can be used when restoring the context
 * saved from the ISR or that saved from a call to vPortYieldProcessor.
 */
void vPortYieldProcessor( void )
{
	/* Within an IRQ ISR the link register has an offset from the true return
	address, but an SWI ISR does not.  Add the offset manually so the same
	ISR return code can be used in both cases. */
	asm volatile ( "ADD		LR, LR, #4" );

	/* Perform the context switch.  First save the context of the current task. */
	portSAVE_CONTEXT();

	/* Find the highest priority task that is ready to run. */
	vTaskSwitchContext();

	/* Restore the context of the new task. */
	portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

/*
 * The ISR used for the scheduler tick.
 */
void vTickISR( void ) __attribute__((naked));
void vTickISR( void )
{
	/* Save the context of the interrupted task. */
	portSAVE_CONTEXT();

	/* Increment the RTOS tick count, then look for the highest priority
	task that is ready to run. */
	vTaskIncrementTick();

#if configUSE_PREEMPTION == 1
	vTaskSwitchContext();
#endif

	/* Ready for the next interrupt. */
	// Clear TCF and set TCFIE.
	TmrStatusCtrl_t statusCtrl;
	TmrGetStatusControl(gTmr0_c, &statusCtrl);
	statusCtrl.bitFields.TCF = 0;
	statusCtrl.bitFields.TCFIE = 1;
	TmrSetStatusControl(gTmr0_c, &statusCtrl);

	// Reset the counter, so we get a full count on the next run.
	SetCntrVal(gTmr0_c, 0);

	/* Restore the context of the new task. */
	portRESTORE_CONTEXT();
}
/*-----------------------------------------------------------*/

/*
 * The interrupt management utilities can only be called from ARM mode.  When
 * THUMB_INTERWORK is defined the utilities are defined as functions here to
 * ensure a switch to ARM mode.  When THUMB_INTERWORK is not defined then
 * the utilities are defined as macros in portmacro.h - as per other ports.
 */
#ifdef THUMB_INTERWORK

	void vPortDisableInterruptsFromThumb( void ) __attribute__ ((naked));
	void vPortEnableInterruptsFromThumb( void ) __attribute__ ((naked));

	void vPortDisableInterruptsFromThumb( void )
	{
		asm volatile (
			"STMDB	SP!, {R0}		\n\t"	/* Push R0.									*/
			"MRS	R0, CPSR		\n\t"	/* Get CPSR.								*/
			"ORR	R0, R0, #0xC0	\n\t"	/* Disable IRQ, FIQ.						*/
			"MSR	CPSR, R0		\n\t"	/* Write back modified value.				*/
			"LDMIA	SP!, {R0}		\n\t"	/* Pop R0.									*/
			"BX		R14" );					/* Return back to thumb.					*/
	}

	void vPortEnableInterruptsFromThumb( void )
	{
		asm volatile (
			"STMDB	SP!, {R0}		\n\t"	/* Push R0.									*/
			"MRS	R0, CPSR		\n\t"	/* Get CPSR.								*/
			"BIC	R0, R0, #0xC0	\n\t"	/* Enable IRQ, FIQ.							*/
			"MSR	CPSR, R0		\n\t"	/* Write back modified value.				*/
			"LDMIA	SP!, {R0}		\n\t"	/* Pop R0.									*/
			"BX		R14" );					/* Return back to thumb.					*/
	}

#endif /* THUMB_INTERWORK */

/* The code generated by the GCC compiler uses the stack in different ways at
different optimisation levels.  The interrupt flags can therefore not always
be saved to the stack.  Instead the critical section nesting level is stored
in a variable, which is then saved as part of the stack context. */
void vPortEnterCritical( void )
{
	/* Disable interrupts as per portDISABLE_INTERRUPTS(); 							*/
	asm volatile (
		"STMDB	SP!, {R0}			\n\t"	/* Push R0.								*/
		"MRS	R0, CPSR			\n\t"	/* Get CPSR.							*/
		"ORR	R0, R0, #0xC0		\n\t"	/* Disable IRQ, FIQ.					*/
		"MSR	CPSR, R0			\n\t"	/* Write back modified value.			*/
		"LDMIA	SP!, {R0}" );				/* Pop R0.								*/

	/* Now interrupts are disabled ulCriticalNesting can be accessed
	directly.  Increment ulCriticalNesting to keep a count of how many times
	portENTER_CRITICAL() has been called. */
	ulCriticalNesting++;
}

void vPortExitCritical( void )
{
	if( ulCriticalNesting > portNO_CRITICAL_NESTING )
	{
		/* Decrement the nesting count as we are leaving a critical section. */
		ulCriticalNesting--;

		/* If the nesting level has reached zero then interrupts should be
		re-enabled. */
		if( ulCriticalNesting == portNO_CRITICAL_NESTING )
		{
			/* Enable interrupts as per portEXIT_CRITICAL().					*/
			asm volatile (
				"STMDB	SP!, {R0}		\n\t"	/* Push R0.						*/
				"MRS	R0, CPSR		\n\t"	/* Get CPSR.					*/
				"BIC	R0, R0, #0xC0	\n\t"	/* Enable IRQ, FIQ.				*/
				"MSR	CPSR, R0		\n\t"	/* Write back modified value.	*/
				"LDMIA	SP!, {R0}" );			/* Pop R0.						*/
		}
	}
}
