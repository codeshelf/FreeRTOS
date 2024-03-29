/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : MC13191.C
**     Project   : RTOSDemo
**     Processor : MC9S08GT60CFD
**     Beantype  : SynchroMaster
**     Version   : Bean 02.213, Driver 01.10, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/10/2006, 4:30 PM
**     Abstract  :
**         This bean "SynchroMaster" implements MASTER part of synchronous
**         serial master-slave communication.
**     Settings  :
**         Synchro type                : MASTER
**
**         Serial channel              : SPI1
**
**         Protocol
**             Init baud rate          : 9_998MHz
**             Clock edge              : rising
**             Width                   : 8 bits (always)
**             Empty character         : 0
**             Empty char. on input    : RECEIVED
**
**         Registers
**             Input buffer            : SPI1D     [002D]
**             Output buffer           : SPI1D     [002D]
**             Control register        : SPI1C1    [0028]
**             Mode register           : SPI1C2    [0029]
**             Baud setting reg.       : SPI1BR    [002A]
**
**
**
**         Used pins                   :
**         ----------------------------------------------------------
**              Function    | On package |    Name
**         ----------------------------------------------------------
**               Input      |     14     |  PTE3_MISO1
**               Output     |     15     |  PTE4_MOSI1
**               Clock      |     16     |  PTE5_SPSCK1
**         ----------------------------------------------------------
**
**     Contents  :
**         RecvChar              - byte MC13191_RecvChar(MC13191_TComData *Chr);
**         SendChar              - byte MC13191_SendChar(MC13191_TComData Chr);
**         CharsInRxBuf          - byte MC13191_CharsInRxBuf(word *Chr);
**         GetCharsInRxBuf       - word MC13191_GetCharsInRxBuf(void);
**         SetShiftClockPolarity - byte MC13191_SetShiftClockPolarity(byte Edge);
**         SetIdleClockPolarity  - byte MC13191_SetIdleClockPolarity(byte Level);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/


/* MODULE MC13191. */

#include "MC13191.h"
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
#include "SWI.h"
#include "PWM1.h"

#define OVERRUN_ERR      0x01          /* Overrun error flag bit   */
#define CHAR_IN_RX       0x08          /* Char is in RX buffer     */
#define FULL_TX          0x10          /* Full transmit buffer     */
#define RUNINT_FROM_TX   0x20          /* Interrupt is in progress */
#define FULL_RX          0x40          /* Full receive buffer      */

static bool EnUser;                    /* Enable/Disable SPI */
static bool EnMode;                    /* Enable/Disable SPI in speed mode */
static byte SerFlag;                   /* Flags for serial communication */
                                       /* Bits: 0 - OverRun error */
                                       /*       1 - Unused */
                                       /*       2 - Unused */
                                       /*       3 - Char in RX buffer */
                                       /*       4 - Full TX buffer */
                                       /*       5 - Running int from TX */
                                       /*       6 - Full RX buffer */
                                       /*       7 - Unused */
static MC13191_TComData BufferWrite;   /* Output char SPI commmunication */

/*
** ===================================================================
**     Method      :  HWEnDi (bean SynchroMaster)
**
**     Description :
**         Enables or disables the peripheral(s) associated with the bean.
**         The method is called automatically as a part of the Enable and 
**         Disable methods and several internal methods.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
static void HWEnDi(void)
{
  if (EnMode && EnUser) {              /* Enable device? */
    SPI1C1_SPE = 1;                    /* Enable device */
    if (SerFlag & FULL_TX) {           /* Is any char in transmit buffer? */
      SPI1D = BufferWrite;             /* Store char to transmitter register */
      SerFlag &= ~FULL_TX;             /* Zeroize FULL_TX flag */
    }
  }
  else {
    SPI1C1_SPE = 0;                    /* Disable device */
  }
}

/*
** ===================================================================
**     Method      :  MC13191_RecvChar (bean SynchroMaster)
**
**     Description :
**         If any data is received, this method returns one
**         character, otherwise it returns an error code (it does
**         not wait for data).
**         DMA mode:
**         If DMA controller is available on selected CPU and
**         receiver is configured to use DMA controller then this
**         method only sets the selected DMA channel. Status of the
**         DMA transfer can then be checked using method
**         GetCharsInRxBuf. See typical usage for details about
**         communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Chr             - A pointer to the received character
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_RXEMPTY - No data in receiver
**                           ERR_OVERRUN - Overrun error was detected
**                           from the last char or block received
**                           ERR_FAULT - Fault error was detected
**                           from the last char or block received.
**                           This error may not be supported on some
**                           CPUs (see generated code).
** ===================================================================
*/
byte MC13191_RecvChar(MC13191_TComData *Chr)
{
  byte Status;

  if (!EnMode)                         /* Is the device disabled in the actual speed CPU mode? */
    return ERR_SPEED;                  /* If yes then error */
  Status = SPI1S;                      /* Read the device error register */
  *Chr = SPI1D;                        /* Read data from receiver */
  if (!(Status & 0x80))                /* Is not received char? */
    return ERR_RXEMPTY;                /* If yes then error is returned */
  return ERR_OK;
}

/*
** ===================================================================
**     Method      :  MC13191_SendChar (bean SynchroMaster)
**
**     Description :
**         Sends one character to the channel.
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the transmitter is configured to use DMA controller then
**         this method only sets the selected DMA channel. The
**         status of the DMA transfer can then be checked using
**         GetCharsInTxBuf method. See the typical usage for details
**         about communication using DMA.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Chr             - Character to send
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED - Device is disabled (only
**                           if output DMA is supported and enabled)
**                           ERR_TXFULL - Transmitter is full
** ===================================================================
*/
byte MC13191_SendChar(MC13191_TComData Chr)
{
  if (!EnMode)                         /* Is the device disabled in the actual speed CPU mode? */
    return ERR_SPEED;                  /* If yes then error */
  if ((!SPI1S_SPTEF)||(SerFlag&FULL_TX)) { /* Is last character send? */
    return ERR_TXFULL;                 /* If no then return error */
  }
  if(EnUser) {                         /* Is device enabled? */
    SPI1D = Chr;                       /* If yes, send character */
  }
  else {
    BufferWrite = Chr;                 /* If no, save character */
    SerFlag |= FULL_TX;                /* ...and set flag */
  }
  return ERR_OK;                       /* OK */
}

/*
** ===================================================================
**     Method      :  MC13191_CharsInRxBuf (bean SynchroMaster)
**
**     Description :
**         Returns the number of characters in the input buffer.
**         Note: If the Interrupt service is disabled, and the
**         Ignore empty character is set to yes, and a character has
**         been received, then this method returns 1 although it was
**         an empty character.
**         (deprecated method)
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this method returns the number of characters in the
**         receive buffer.
**     Parameters  :
**         NAME            - DESCRIPTION
**       * Chr             - A pointer to number of characters in
**                           the input buffer
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
** ===================================================================
*/
byte MC13191_CharsInRxBuf(word *Chr)
{
  if (!EnMode)                         /* Is the device disabled in the actual speed CPU mode? */
    return ERR_SPEED;                  /* If yes then error */
  *Chr = SPI1S_SPRF;                   /* Return number of chars in receive buffer */
  return ERR_OK;                       /* OK */
}

/*
** ===================================================================
**     Method      :  MC13191_GetCharsInRxBuf (bean SynchroMaster)
**
**     Description :
**         Returns the number of characters in the input buffer.
**         Note: If the Interrupt service is disabled, and the
**         Ignore empty character is set to yes, and a character has
**         been received, then this method returns 1 although it was
**         an empty character.
**         DMA mode:
**         If DMA controller is available on the selected CPU and
**         the receiver is configured to use DMA controller then
**         this method returns the number of characters in the
**         receive buffer.
**     Parameters  : None
**     Returns     :
**         ---             - Number of characters in the input
**                           buffer.
** ===================================================================
*/
/*
word MC13191_GetCharsInRxBuf(void)

**      This method is implemented as a macro. See header module. **
*/

/*
** ===================================================================
**     Method      :  MC13191_SetShiftClockPolarity (bean SynchroMaster)
**
**     Description :
**         Sets the shift clock polarity at runtime. Output data
**         will be shifted on the selected edge polarity.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Edge            - Edge polarity.
**                           0-falling edge
**                           1-rising edge
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED - Device is disabled
**                           ERR_RANGE - Parameter out of range
** ===================================================================
*/
byte MC13191_SetShiftClockPolarity(byte Edge)
{
  if(!EnMode)
    return ERR_SPEED;
  if(Edge > 1)
    return ERR_RANGE;
  EnUser=FALSE;
  HWEnDi();                            /* Disable device */
  SPI1C1_CPHA = SPI1C1_CPOL ^ Edge;    /* Set phase bit */
  EnUser=TRUE;
  HWEnDi();                            /* Enable device */
  return ERR_OK;
}

/*
** ===================================================================
**     Method      :  MC13191_SetIdleClockPolarity (bean SynchroMaster)
**
**     Description :
**         Sets the idle clock polarity at runtime. If the
**         communication does not run, the clock signal will have
**         required level.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Level           - Idle clock polarity:
**                           0-low
**                           1-high
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_DISABLED - Device is disabled
**                           ERR_RANGE - Parameter out of range
** ===================================================================
*/
byte MC13191_SetIdleClockPolarity(byte Level)
{
  if(!EnMode)
    return ERR_SPEED;
  if(Level > 1)
    return ERR_RANGE;
  if(SPI1C1_CPOL != Level) {
    PTED_PTED5 = Level;                /* Define level on clk pin when device is disabled */
    EnUser=FALSE;
    HWEnDi();                          /* Disable device */
    SPI1C1 ^= 0x0C;                    /* Invert polarity and phase bit */
    EnUser=TRUE;
    HWEnDi();                          /* Enable device */
  }
  return ERR_OK;
}

/*
** ===================================================================
**     Method      :  MC13191_Init (bean SynchroMaster)
**
**     Description :
**         Initializes the associated peripheral(s) and the bean's 
**         internal variables. The method is called automatically as a 
**         part of the application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void MC13191_Init(void)
{
  SerFlag = 0;                         /* Reset all flags */
  EnUser = TRUE;                       /* Enable device */
  #pragma MESSAGE DISABLE C4002        /* Disable warning C4002 "Result not used" */
  SPI1S;                               /* Read the status register */
  SPI1D;                               /* Read the device register */
  /* SPI1BR: ??=0,SPPR2=0,SPPR1=0,SPPR0=0,??=0,SPR2=0,SPR1=0,SPR0=0 */
  setReg8(SPI1BR, 0x00);               /* Set the baud rate register */ 
  /* SPI1C2: ??=0,??=0,??=0,MODFEN=0,BIDIROE=0,??=0,SPISWAI=0,SPC0=0 */
  setReg8(SPI1C2, 0x00);               /* Configure the SPI port - control register 2 */ 
  /* SPI1C1: SPIE=0,SPE=0,SPTIE=0,MSTR=1,CPOL=0,CPHA=1,SSOE=0,LSBFE=0 */
  setReg8(SPI1C1, 0x14);               /* Configure the SPI port - control register 1 */ 
  MC13191_SetHigh();                   /* Initial speed CPU mode is high */
}

/*
** ===================================================================
**     Method      :  MC13191_SetHigh (bean SynchroMaster)
**
**     Description :
**         The method reconfigures the bean and its selected peripheral(s)
**         when the CPU is switched to the High speed mode. The method is 
**         called automatically as s part of the CPU SetHighSpeed method.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void MC13191_SetHigh(void)
{
  EnMode = TRUE;                       /* Set the flag "device enabled" in the actual speed CPU mode */
  HWEnDi();                            /* Enable/disable device according to the status flags */
}

/*
** ===================================================================
**     Method      :  MC13191_SetLow (bean SynchroMaster)
**
**     Description :
**         The method reconfigures the bean and its selected peripheral(s)
**         when the CPU is switched to the Low speed mode. The method is 
**         called automatically as a part of the CPU SetLowSpeed method.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void MC13191_SetLow(void)
{
  EnMode = FALSE;                      /* Set the flag "device disabled" in the actual speed CPU mode */
  HWEnDi();                            /* Enable/disable device according to the status flags */
}


/* END MC13191. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
