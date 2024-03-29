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

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "portable.h"

/* Processor Expert created headers. */
#include "led1.h"
#include "led2.h"
#include "led3.h"
#include "led4.h"

/* Demo application include files. */
#include "partest.h"

/*-----------------------------------------------------------
 * Simple parallel port IO routines.
 *-----------------------------------------------------------*/

void vParTestSetLED( unsigned portBASE_TYPE uxLED, signed portBASE_TYPE xValue )
{
	/* This function is required as it is called from the standard demo 
	application files.  All it does however is call the Processor Expert
	created function. */
	portENTER_CRITICAL();
		//Byte1_PutBit( uxLED, !xValue );
		switch (uxLED) {
			case 1:
				LED1_SetVal();
				break;
			case 2:
				LED2_SetVal();
				break;
			case 3:
				LED3_SetVal();
				break;
			case 4:
				LED4_SetVal();
				break;
		}
		
	portEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

void vParTestToggleLED( unsigned portBASE_TYPE uxLED )
{
	/* This function is required as it is called from the standard demo
	application files.  All it does however is call the processor Expert
	created function. */
	portENTER_CRITICAL();
		switch (uxLED) {
			case 1:
				LED1_NegVal();
				break;
			case 2:
				LED2_NegVal();
				break;
			case 3:
				LED3_NegVal();
				break;
			case 4:
				LED4_NegVal();
				break;
		}
	portEXIT_CRITICAL();
}



