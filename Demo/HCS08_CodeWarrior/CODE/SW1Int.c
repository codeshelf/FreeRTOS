/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : SW1Int.C
**     Project   : RTOSDemo
**     Processor : MC9S08GT60CFD
**     Beantype  : ExtInt
**     Version   : Bean 02.066, Driver 01.08, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/10/2006, 4:30 PM
**     Abstract  :
**         This bean "ExtInt" implements an external 
**         interrupt, its control methods and interrupt/event 
**         handling procedure.
**         The bean uses one pin which generates interrupt on 
**         selected edge.
**     Settings  :
**         Interrupt name              : Vkeyboard1
**         User handling procedure     : SW1Int_OnInterrupt
**
**         Used pin                    :
**             ----------------------------------------------------
**                Number (on package)  |    Name
**             ----------------------------------------------------
**                       37            |  PTA2_KBI1P2
**             ----------------------------------------------------
**
**         Port name                   : PTA
**
**         Bit number (in port)        : 2
**         Bit mask of the port        : 0004
**
**         Signal edge/level           : low
**         Pull option                 : up
**         Initial state               : Enabled
**
**         Edge register               : KBI1SC    [0016]
**         Enable register             : KBI1PE    [0017]
**         Request register            : KBI1SC    [0016]
**
**         Port data register          : PTAD      [0000]
**         Port control register       : PTADD     [0003]
**         Port function register      : KBI1PE    [0017]
**     Contents  :
**         Enable - void SW1Int_Enable(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#include "SW1Int.h"

/*Including shared modules, which are used for all project*/
#include "Events.h"
#include "Cpu.h"

/*
** ===================================================================
**     Method      :  SW1Int_Enable (bean ExtInt)
**
**     Description :
**         Enable the bean - the external events are accepted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void SW1Int_Enable(void)
{
  KBI1SC_KBACK = 0x01;                 /* Clear IRQ flag */
  KBI1SC_KBIE = 0x01;                  /* Enable device */
}

/*
** ===================================================================
**     Method      :  SW1Int_Interrupt (bean ExtInt)
**
**     Description :
**         The method services the interrupt of the selected peripheral(s)
**         and eventually invokes the beans event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
ISR(SW1Int_Interrupt)
{
  KBI1SC_KBACK = 1;                    /* Clear the interrupt flag */
  SW1Int_OnInterrupt();
}

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
