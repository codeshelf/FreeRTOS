/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : LED2.H
**     Project   : RTOSDemo
**     Processor : MC9S08GT60CFD
**     Beantype  : BitIO
**     Version   : Bean 02.066, Driver 03.08, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/10/2006, 4:30 PM
**     Abstract  :
**         This bean "BitIO" implements an one-bit input/output.
**         It uses one bit/pin of a port.
**         Methods of this bean are mostly implemented as a macros
**         (if supported by target language and compiler).
**     Settings  :
**         Used pin                    :
**             ----------------------------------------------------
**                Number (on package)  |    Name
**             ----------------------------------------------------
**                       21            |  PTD1_TPM1CH1
**             ----------------------------------------------------
**
**         Port name                   : PTD
**
**         Bit number (in port)        : 1
**         Bit mask of the port        : 0002
**
**         Initial direction           : Output (direction can be changed)
**         Safe mode                   : yes
**         Initial output value        : 1
**         Initial pull option         : off
**
**         Port data register          : PTDD      [000C]
**         Port control register       : PTDDD     [000F]
**
**         Optimization for            : speed
**     Contents  :
**         SetDir - void LED2_SetDir(bool Dir);
**         ClrVal - void LED2_ClrVal(void);
**         SetVal - void LED2_SetVal(void);
**         NegVal - void LED2_NegVal(void);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

#ifndef LED2_H_
#define LED2_H_

/* MODULE LED2. */

  /* Including shared modules, which are used in the whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PE_Timer.h"
#include "Cpu.h"


/*
** ===================================================================
**     Method      :  LED2_ClrVal (bean BitIO)
**
**     Description :
**         This method clears (sets to zero) the output value.
**           a) direction = Input  : sets the output value to "0";
**                                   this operation will be shown on
**                                   output after the direction has
**                                   been switched to output
**                                   (SetDir(TRUE);)
**           b) direction = Output : directly writes "0" to the
**                                   appropriate pin
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LED2_ClrVal() ( \
    (void)clrReg8Bits(PTDD, 0x02)      /* PTDD1=0x00 */, \
    Shadow_PTD &= ~0x02                /* Set appropriate bit in shadow variable */ \
  )

/*
** ===================================================================
**     Method      :  LED2_SetVal (bean BitIO)
**
**     Description :
**         This method sets (sets to one) the output value.
**           a) direction = Input  : sets the output value to "1";
**                                   this operation will be shown on
**                                   output after the direction has
**                                   been switched to output
**                                   (SetDir(TRUE);)
**           b) direction = Output : directly writes "1" to the
**                                   appropriate pin
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LED2_SetVal() ( \
    (void)setReg8Bits(PTDD, 0x02)      /* PTDD1=0x01 */, \
    Shadow_PTD |= 0x02                 /* Set appropriate bit in shadow variable */ \
  )

/*
** ===================================================================
**     Method      :  LED2_NegVal (bean BitIO)
**
**     Description :
**         This method negates (inverts) the output value.
**           a) direction = Input  : inverts the output value;
**                                   this operation will be shown on
**                                   output after the direction has
**                                   been switched to output
**                                   (SetDir(TRUE);)
**           b) direction = Output : directly inverts the value
**                                   of the appropriate pin
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
#define LED2_NegVal() ( \
    (void)invertReg8Bits(PTDD, 0x02)   /* PTDD1=invert */, \
    Shadow_PTD ^= 0x02                 /* Set appropriate bit in shadow variable */ \
  )

/*
** ===================================================================
**     Method      :  LED2_SetDir (bean BitIO)
**
**     Description :
**         This method sets direction of the bean.
**     Parameters  :
**         NAME       - DESCRIPTION
**         Dir        - Direction to set (FALSE or TRUE)
**                      FALSE = Input, TRUE = Output
**     Returns     : Nothing
** ===================================================================
*/
void LED2_SetDir(bool Dir);



/* END LED2. */
#endif /* #ifndef __LED2_H_ */
/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
