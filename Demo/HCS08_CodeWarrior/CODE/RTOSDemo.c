/* MODULE RTOSDemo */

/* Including used modules for compilling procedure */
#include "Cpu.h"
#include "Events.h"
#include "TickTimer.h"
#include "SW1Int.h"
#include "SW2.h"
#include "SW3.h"
#include "SW4.h"
#include "LED1.h"
#include "LED2.h"
#include "LED3.h"
#include "LED4.h"
#include "UART.h"
#include "USB.h"
#include "MC13191.h"
#include "SWI.h"
#include "PWM1.h"
/* Include shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

extern void vMain( void );

void main(void)
{
  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /*Write your code here*/
  
  /* Just jump to the real main(). */
  __asm
  {
  	 jmp vMain
  }
  
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END RTOSDemo */
