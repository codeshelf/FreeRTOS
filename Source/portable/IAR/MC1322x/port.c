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
 * Implementation of functions defined in portable.h for the Philips ARM7 port.
 *----------------------------------------------------------*/

/*
 Changes from V3.2.2

 + Bug fix - The prescale value for the timer setup is now written to T0PR
 instead of T0PC.  This bug would have had no effect unless a prescale
 value was actually used.
 */

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "Leds.h"

/* Constants required to setup the tick ISR. */
#define portENABLE_TIMER			( ( unsigned portCHAR ) 0x01 )
#define portINTERRUPT_ON_MATCH		( ( unsigned portLONG ) 0x01 )
#define portRESET_COUNT_ON_MATCH	( ( unsigned portLONG ) 0x02 )

/* Constants required to setup the initial stack. */
#define portINITIAL_SPSR				( ( portSTACK_TYPE ) 0x3f ) /* System mode, THUMB mode, interrupts enabled. */
#define portINSTRUCTION_SIZE			( ( portSTACK_TYPE ) 4 )

/* Constants required to setup the PIT. */
#define portPIT_CLOCK_DIVISOR			( ( unsigned portLONG ) 16 )
#define portPIT_COUNTER_VALUE			( ( ( configCPU_CLOCK_HZ / portPIT_CLOCK_DIVISOR ) / 1000UL ) * portTICK_RATE_MS )

/* Constants required to handle interrupts. */
#define portTIMER_MATCH_ISR_BIT		( ( unsigned portCHAR ) 0x01 )
#define portCLEAR_VIC_INTERRUPT		( ( unsigned portLONG ) 0 )

/* Constants required to handle critical sections. */
#define portNO_CRITICAL_NESTING 		( ( unsigned portLONG ) 0 )

#define portINT_LEVEL_SENSITIVE  0
#define portPIT_ENABLE      	( ( unsigned portSHORT ) 0x1 << 24 )
#define portPIT_INT_ENABLE     	( ( unsigned portSHORT ) 0x1 << 25 )

/* Constants required to setup the VIC for the tick ISR. */
#define portTIMER_VIC_CHANNEL		( ( unsigned portLONG ) 0x0004 )
#define portTIMER_VIC_CHANNEL_BIT	( ( unsigned portLONG ) 0x0010 )
#define portTIMER_VIC_ENABLE		( ( unsigned portLONG ) 0x0020 )

/*-----------------------------------------------------------*/
extern void vPortPreemptiveTickEntry(void);

/* Setup the PIT to generate the tick interrupts. */
static void prvSetupTimerInterrupt(void);

/* ulCriticalNesting will get set to zero when the first task starts.  It
 cannot be initialised to 0 as this will cause interrupts to be enabled
 during the kernel initialisation process. */
unsigned portLONG ulCriticalNesting = (unsigned portLONG ) 9999;

/*-----------------------------------------------------------*/

/*
 * Initialise the stack of a task to look exactly as if a call to
 * portSAVE_CONTEXT had been called.
 *
 * See header file for description.
 */
portSTACK_TYPE *pxPortInitialiseStack(portSTACK_TYPE *pxTopOfStack, pdTASK_CODE pxCode, void *pvParameters) {
	portSTACK_TYPE *pxOriginalTOS;

	pxOriginalTOS = pxTopOfStack;

	/* Setup the initial stack of the task.  The stack is set exactly as
	 expected by the portRESTORE_CONTEXT() macro. */

	/* First on the stack is the return address - which in this case is the
	 start of the task.  The offset is added to make the return address appear
	 as it would within an IRQ ISR. */
	*pxTopOfStack = (portSTACK_TYPE ) pxCode + portINSTRUCTION_SIZE;
	pxTopOfStack--;

	// Nope, we want the current LR (which is pxTopOfStack - 56
	//*pxTopOfStack = (portSTACK_TYPE ) 0xaaaaaaaa; /* R14 */
	*pxTopOfStack = (portSTACK_TYPE) pxTopOfStack - 56;
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) pxOriginalTOS; /* Stack used when task starts goes in R13. */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x12121212; /* R12 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x11111111; /* R11 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x10101010; /* R10 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x09090909; /* R9 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x08080808; /* R8 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x07070707; /* R7 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x06060606; /* R6 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x05050505; /* R5 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x04040404; /* R4 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x03030303; /* R3 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x02020202; /* R2 */
	pxTopOfStack--;
	*pxTopOfStack = (portSTACK_TYPE ) 0x01010101; /* R1 */
	pxTopOfStack--;

	/* When the task starts is will expect to find the function parameter in
	 R0. */
	*pxTopOfStack = (portSTACK_TYPE ) pvParameters; /* R0 */
	pxTopOfStack--;

	/* The status register is set for system mode, with interrupts enabled. */
	*pxTopOfStack = (portSTACK_TYPE) portINITIAL_SPSR;
	pxTopOfStack--;

	/* Interrupt flags cannot always be stored on the stack and will
	 instead be stored in a variable, which is then saved as part of the
	 tasks context. */
	*pxTopOfStack = portNO_CRITICAL_NESTING;

	return pxTopOfStack;
}
	/*-----------------------------------------------------------*/

portBASE_TYPE xPortStartScheduler(void) {
	extern void
	vPortStartFirstTask(void);

	/* Start the timer that generates the tick ISR.  Interrupts are disabled
	 here already. */
	prvSetupTimerInterrupt();

	/* Start the first task. */
	vPortStartFirstTask();

	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler(void) {
	/* It is unlikely that the ARM port will require this function as there
	 is nothing to return to.  */
}
/*-----------------------------------------------------------*/

#if configUSE_PREEMPTION == 0

/* The cooperative scheduler requires a normal IRQ service routine to
 simply increment the system tick. */
static __arm __irq void vPortNonPreemptiveTick( void );
static __arm __irq void vPortNonPreemptiveTick( void )
{
	/* Increment the tick count - which may wake some tasks but as the
	 preemptive scheduler is not being used any woken task is not given
	 processor time no matter what its priority. */
	vTaskIncrementTick();

	/* Ready for the next interrupt. */
	// Clear TCF and set TCFIE.
	TmrStatusCtrl_t statusCtrl;
	TmrGetStatusControl(gTmr0_c, &statusCtrl);
	statusCtrl.bitFields.TCF = 0;
	statusCtrl.bitFields.TCFIE = 1;
	TmrSetStatusControl(gTmr0_c, &statusCtrl);

	// Reset the counter, so we get a full count on the next run.
	SetCntrVal(gTmr0_c, 0);
}

#else

/* This function is called from an asm wrapper, so does not require the __irq
 keyword. */
void vPortPreemptiveTick(void);
void vPortPreemptiveTick(void) {
	TmrErr_t error;

	/* Increment the tick counter. */
	vTaskIncrementTick();

	/* The new tick value might unblock a task.  Ensure the highest task that
	 is ready to execute is the task that will execute when the tick ISR
	 exits. */
	vTaskSwitchContext();

	/* Ready for the next interrupt. */
	// Reset the timer and clear the TCF.

//	// Don't stat the timer yet.
//	TmrSetMode(gTmr0_c, gTmrNoOperation_c);
//
//	// Reset the counter, so we get a full count on the next run.
//	SetCntrVal(gTmr0_c, 0);

	// Clear TCF, TOF, IEF and set TCFIE.
	TmrStatusCtrl_t statusCtrl;
	error = TmrGetStatusControl(gTmr0_c, &statusCtrl);
	statusCtrl.bitFields.TCF = 0;
	statusCtrl.bitFields.TOF = 0;
	statusCtrl.bitFields.IEF = 0;
	//statusCtrl.bitFields.TCFIE = 0;
	error = TmrSetStatusControl(gTmr0_c, &statusCtrl);

//	TmrComparatorStatusCtrl_t tmrComparatorStatusCtrl;
//	TmrGetCompStatusControl(gTmr0_c, &tmrComparatorStatusCtrl);
//	tmrComparatorStatusCtrl.bitFields.TCF1 = 0;
//	tmrComparatorStatusCtrl.bitFields.TCF2 = 0;
//	TmrSetCompStatusControl(gTmr0_c, &tmrComparatorStatusCtrl);
//
//	TmrSetMode(gTmr0_c, gTmrCntRiseEdgPriSrc_c);
}

#endif

/*-----------------------------------------------------------*/

static void prvTmrCallback(TmrNumber_t tmrNumber);
static void prvTmrCallback(TmrNumber_t tmrNumber) {
	// When the timer interrupts we need to dispatch to supervisor mode to manage the OS.
	//asm ("SWI 0");
	/* The ISR installed depends on whether the preemptive or cooperative scheduler is being used. */
#if configUSE_PREEMPTION == 1
	vPortPreemptiveTickEntry();
#else
	vPortNonPreemptiveTick();
#endif
}

/*-----------------------------------------------------------*/

static void prvSetupTimerInterrupt(void) {

	TmrConfig_t tmrConfig;
	TmrStatusCtrl_t tmrStatusCtrl;
	TmrComparatorStatusCtrl_t tmrComparatorStatusCtrl;

	/* Enable hw timer 0 */
	TmrEnable(gTmr0_c);
	/* Don't stat the timer yet */
	TmrSetMode(gTmr0_c, gTmrNoOperation_c);

	/* Register the callback executed when a timer interrupt occur */
	TmrSetCallbackFunction(gTmr0_c, gTmrCompEvent_c, prvTmrCallback);

	/* The timer interrupt with then throw an SWI, so provide a handler for that too. */
	//IntAssignHandler(gSupervisorException_c, vPortYieldProcessor);

	tmrStatusCtrl.uintValue = 0x0000;
	tmrStatusCtrl.bitFields.OEN = TRUE;;
	tmrStatusCtrl.bitFields.TCFIE = TRUE;
	TmrSetStatusControl(gTmr0_c, &tmrStatusCtrl);

	tmrComparatorStatusCtrl.uintValue = 0x0000;
	tmrComparatorStatusCtrl.bitFields.DBG_EN = 0x01;
	tmrComparatorStatusCtrl.bitFields.TCF1EN = FALSE;
	//tmrComparatorStatusCtrl.bitFields.CL1 = 0x01;
	TmrSetCompStatusControl(gTmr0_c, &tmrComparatorStatusCtrl);

	tmrConfig.tmrOutputMode = gTmrToggleOF_c;
	tmrConfig.tmrCoInit = FALSE; /*co-chanel counter/timers can not force a re-initialization of this counter/timer*/
	tmrConfig.tmrCntDir = FALSE; /*count-up*/
	tmrConfig.tmrCntLen = TRUE; /*count until compare*/
	tmrConfig.tmrCntOnce = FALSE; /*count repeatedly*/
	tmrConfig.tmrSecondaryCntSrc = gTmrSecondaryCnt0Input_c; /*secondary count source not needed*/
	tmrConfig.tmrPrimaryCntSrc = gTmrPrimaryClkDiv1_c; /*primary count source is IP BUS clock divide by 8 prescaler*/
	TmrSetConfig(gTmr0_c, &tmrConfig);

	/* Config timer to raise interrupts each 1 ms */
	SetComp1Val(gTmr0_c, (configCPU_CLOCK_HZ / configTICK_RATE_HZ));
	SetCompLoad1Val(gTmr0_c, (configCPU_CLOCK_HZ / configTICK_RATE_HZ));

	/* Config timer to start from 0 after compare event */
	SetLoadVal(gTmr0_c, 0);

	/* Start the counter at 0. */
	SetCntrVal(gTmr0_c, 0);

	/* Setup the interrupt handling to catch the TMR0 interrupts. */
	IntAssignHandler(gTmrInt_c, (IntHandlerFunc_t) TmrIsr);
	ITC_SetPriority(gTmrInt_c, gItcNormalPriority_c);
	ITC_EnableInterrupt(gTmrInt_c);

	TmrSetMode(gTmr0_c, gTmrCntRiseEdgPriSrc_c);
}

/*-----------------------------------------------------------*/

void vPortEnterCritical(void) {
	/* Disable interrupts first! */
	portDISABLE_INTERRUPTS();

	/* Now interrupts are disabled ulCriticalNesting can be accessed
	 directly.  Increment ulCriticalNesting to keep a count of how many times
	 portENTER_CRITICAL() has been called. */
	ulCriticalNesting++;
}
/*-----------------------------------------------------------*/

void vPortExitCritical(void) {
	if (ulCriticalNesting > portNO_CRITICAL_NESTING) {
		/* Decrement the nesting count as we are leaving a critical section. */
		ulCriticalNesting--;

		/* If the nesting level has reached zero then interrupts should be
		 re-enabled. */
		if (ulCriticalNesting == portNO_CRITICAL_NESTING) {
			portENABLE_INTERRUPTS();
		}
	}
}
/*-----------------------------------------------------------*/

