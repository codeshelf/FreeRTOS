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
 * Implementation of functions defined in portable.h for the ARM7 port.
 *
 * Components that can be compiled to either ARM or THUMB mode are
 * contained in this file.  The ISR routines, which can only be compiled
 * to ARM mode are contained in portISR.c.
 *----------------------------------------------------------*/

/*
	Changes from V2.5.2

	+ ulCriticalNesting is now saved as part of the task context, as is
	  therefore added to the initial task stack during pxPortInitialiseStack.

	Changes from V3.2.2

	+ Bug fix - The prescale value for the timer setup is now written to T0_PR
	  instead of T0_PC.  This bug would have had no effect unless a prescale
	  value was actually used.
*/


/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Constants required to setup the task context. */
#define portINITIAL_SPSR				( ( portSTACK_TYPE ) 0x1f ) /* System mode, ARM mode, interrupts enabled. */
#define portTHUMB_MODE_BIT				( ( portSTACK_TYPE ) 0x20 )
#define portINSTRUCTION_SIZE			( ( portSTACK_TYPE ) 4 )
#define portNO_CRITICAL_SECTION_NESTING	( ( portSTACK_TYPE ) 0 )

/* Constants required to setup the tick ISR. */
#define portENABLE_TIMER			( ( unsigned portCHAR ) 0x01 )
#define portPRESCALE_VALUE			0x00
#define portINTERRUPT_ON_MATCH		( ( unsigned portLONG ) 0x01 )
#define portRESET_COUNT_ON_MATCH	( ( unsigned portLONG ) 0x02 )

/* Constants required to setup the VIC for the tick ISR. */
#define portTIMER_VIC_CHANNEL		( ( unsigned portLONG ) 0x0004 )
#define portTIMER_VIC_CHANNEL_BIT	( ( unsigned portLONG ) 0x0010 )
#define portTIMER_VIC_ENABLE		( ( unsigned portLONG ) 0x0020 )

/*-----------------------------------------------------------*/

/* Setup the timer to generate the tick interrupts. */
static void prvSetupTimerInterrupt( void );

/*
 * The scheduler can only be started from ARM mode, so
 * vPortISRStartFirstSTask() is defined in portISR.c.
 */
extern void vPortISRStartFirstTask( void );

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack( portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters )
{
portSTACK_TYPE *pxOriginalTOS;

	pxOriginalTOS = pxTopOfStack;

	/* Setup the initial stack of the task.  The stack is set exactly as
	expected by the portRESTORE_CONTEXT() macro. */

	/* First on the stack is the return address - which in this case is the
	start of the task.  The offset is added to make the return address appear
	as it would within an IRQ ISR. */
	*pxTopOfStack = ( portSTACK_TYPE ) pxCode + portINSTRUCTION_SIZE;
	pxTopOfStack--;

	// Nope, we want the current LR (which is pxTopOfStack - 56
	//*pxTopOfStack = (portSTACK_TYPE ) 0xaaaaaaaa; /* R14 */
	*pxTopOfStack = (portSTACK_TYPE) pxTopOfStack - 56;
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) pxOriginalTOS; /* Stack used when task starts goes in R13. */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x12121212;	/* R12 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x11111111;	/* R11 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x10101010;	/* R10 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x09090909;	/* R9 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x08080808;	/* R8 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x07070707;	/* R7 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x06060606;	/* R6 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x05050505;	/* R5 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x04040404;	/* R4 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x03030303;	/* R3 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x02020202;	/* R2 */
	pxTopOfStack--;
	*pxTopOfStack = ( portSTACK_TYPE ) 0x01010101;	/* R1 */
	pxTopOfStack--;

	/* When the task starts is will expect to find the function parameter in
	R0. */
	*pxTopOfStack = ( portSTACK_TYPE ) pvParameters; /* R0 */
	pxTopOfStack--;

	/* The last thing onto the stack is the status register, which is set for
	system mode, with interrupts enabled. */
	*pxTopOfStack = ( portSTACK_TYPE ) portINITIAL_SPSR;

	#ifdef THUMB_INTERWORK
	{
		/* We want the task to start in thumb mode. */
		*pxTopOfStack |= portTHUMB_MODE_BIT;
	}
	#endif

	pxTopOfStack--;

	/* Some optimisation levels use the stack differently to others.  This
	means the interrupt flags cannot always be stored on the stack and will
	instead be stored in a variable, which is then saved as part of the
	tasks context. */
	*pxTopOfStack = portNO_CRITICAL_SECTION_NESTING;

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

portBASE_TYPE xPortStartScheduler( void )
{
	/* Start the timer that generates the tick ISR.  Interrupts are disabled
	here already. */
	prvSetupTimerInterrupt();

	/* Start the first task. */
	vPortISRStartFirstTask();

	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* It is unlikely that the ARM port will require this function as there
	is nothing to return to.  */
}
/*-----------------------------------------------------------*/

static void prvTmrCallback(TmrNumber_t tmrNumber);
static void prvTmrCallback(TmrNumber_t tmrNumber) {
	// When the timer interrupts we need to dispatch to supervisor mode to manage the OS.
	//asm ("SWI 0");
	/* The ISR installed depends on whether the preemptive or cooperative scheduler is being used. */
#if configUSE_PREEMPTION == 1
	vTickISR();
#else
	vPortNonPreemptiveTick();
#endif
}

/*-----------------------------------------------------------*/

static void prvSWICallback(void);
static void prvSWICallback(void) {
/* The ISR installed depends on whether the preemptive or cooperative scheduler is being used. */
#if configUSE_PREEMPTION == 1
	vTickISR();
#else
	vPortNonPreemptiveTick();
#endif
}

/*-----------------------------------------------------------*/

/*
 * Setup the timer 0 to generate the tick interrupts at the required frequency.
 */
static void prvSetupTimerInterrupt( void )
{
	TmrConfig_t tmrConfig;
	TmrStatusCtrl_t tmrStatusCtrl;
	TmrComparatorStatusCtrl_t tmrComparatorStatusCtrl;

	/* Enable hw timer 1 */
	TmrEnable(gTmr0_c);
	/* Don't stat the timer yet */
	TmrSetMode(gTmr0_c, gTmrNoOperation_c);

	/* Register the callback executed when a timer interrupt occur */
	TmrSetCallbackFunction(gTmr0_c, gTmrComp1Event_c, prvTmrCallback);

	/* The timer interrupt with then throw an SWI, so provide a handler for that too. */
	//IntAssignHandler(gSupervisorException_c, prvSWICallback);

	tmrStatusCtrl.uintValue = 0x0000;
	tmrStatusCtrl.bitFields.OEN = 1;
	TmrSetStatusControl(gTmr0_c, &tmrStatusCtrl);

	tmrComparatorStatusCtrl.uintValue = 0x0000;
	tmrComparatorStatusCtrl.bitFields.DBG_EN = 0x01;
	tmrComparatorStatusCtrl.bitFields.TCF1EN = TRUE;
	tmrComparatorStatusCtrl.bitFields.CL1 = TRUE;
	TmrSetCompStatusControl(gTmr0_c, &tmrComparatorStatusCtrl);

	tmrConfig.tmrOutputMode = gTmrAssert_c;
	tmrConfig.tmrCoInit = FALSE; /*co-chanel counter/timers can not force a re-initialization of this counter/timer*/
	tmrConfig.tmrCntDir = FALSE; /*count-up*/
	tmrConfig.tmrCntLen = TRUE; /*count until compare*/
	tmrConfig.tmrCntOnce = FALSE; /*count repeatedly*/
	tmrConfig.tmrSecondaryCntSrc = gTmrSecondaryCnt0Input_c; /*secondary count source not needed*/
	tmrConfig.tmrPrimaryCntSrc = gTmrPrimaryClkDiv4_c; /*primary count source is IP BUS clock divide by 8 prescaler*/
	TmrSetConfig(gTmr0_c, &tmrConfig);

	/* Config timer to raise interrupts each 1 ms */
	SetComp1Val(gTmr0_c, (configCPU_CLOCK_HZ / configTICK_RATE_HZ));
	SetCompLoad1Val(gTmr0_c, (configCPU_CLOCK_HZ / configTICK_RATE_HZ));

	/* Config timer to start from 0 after compare event */
	SetLoadVal(gTmr0_c, 0);

	/* Start the counter at 0. */
	SetCntrVal(gTmr0_c, 0);

	TmrSetMode(gTmr0_c, gTmrCntRiseEdgPriSrc_c);

	/* Setup the interrupt handling to catch the TMR0 interrupts. */
//	IntAssignHandler(gTmrInt_c, (IntHandlerFunc_t) TmrIsr);
//	ITC_SetPriority(gTmrInt_c, gItcNormalPriority_c);
	ITC_EnableInterrupt(gTmrInt_c);
}

