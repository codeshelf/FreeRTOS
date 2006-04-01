/** ###################################################################
**     THIS BEAN MODULE IS GENERATED BY THE TOOL. DO NOT MODIFY IT.
**     Filename  : TickTimer.C
**     Project   : RTOSDemo
**     Processor : MC9S08GT60CFD
**     Beantype  : TimerInt
**     Version   : Bean 02.110, Driver 01.08, CPU db: 2.87.074
**     Compiler  : Metrowerks HCS08 C Compiler
**     Date/Time : 3/10/2006, 4:30 PM
**     Abstract  :
**         This bean "TimerInt" implements a periodic interrupt.
**         When the bean and its events are enabled, the "OnInterrupt"
**         event is called periodically with the period that you specify.
**         TimerInt supports also changing the period in runtime.
**         The source of periodic interrupt can be timer compare or reload
**         register or timer-overflow interrupt (of free running counter).
**     Settings  :
**         Timer name                  : TPM2 (16-bit)
**         Compare name                : TPM21
**         Counter shared              : No
**
**         High speed mode
**             Prescaler               : divide-by-4
**             Clock                   : 4998857 Hz
**           Initial period/frequency
**             Xtal ticks              : 4000
**             microseconds            : 1000
**             milliseconds            : 1
**             seconds (real)          : 0.0010000
**             Hz                      : 1000
**             kHz                     : 1
**
**         Runtime setting             : period/frequency interval (continual setting)
**             ticks                   : 8 to 40000 ticks
**             microseconds            : 2 to 10000 microseconds
**             milliseconds            : 1 to 10 milliseconds
**             seconds (real)          : 0.0000018 to 0.0100001 seconds
**             Hz                      : 100 to 555428 Hz
**             kHz                     : 1 to 555 kHz
**
**         Initialization:
**              Timer                  : Disabled
**              Events                 : Enabled
**
**         Timer registers
**              Counter                : TPM2CNT   [0061]
**              Mode                   : TPM2SC    [0060]
**              Run                    : TPM2SC    [0060]
**              Prescaler              : TPM2SC    [0060]
**
**         Compare registers
**              Compare                : TPM2C1V   [0069]
**
**         Flip-flop registers
**              Mode                   : TPM2C1SC  [0068]
**     Contents  :
**         Enable    - byte TickTimer_Enable(void);
**         SetFreqHz - byte TickTimer_SetFreqHz(word Freq);
**
**     (c) Copyright UNIS, spol. s r.o. 1997-2005
**     UNIS, spol. s r.o.
**     Jundrovska 33
**     624 00 Brno
**     Czech Republic
**     http      : www.processorexpert.com
**     mail      : info@processorexpert.com
** ###################################################################*/

/* MODULE TickTimer. */

#include "Events.h"
#include "PE_Error.h"
#include "TickTimer.h"
#pragma MESSAGE DISABLE C2705          /* WARNING C2705: Possible loss of data */
#pragma MESSAGE DISABLE C5919          /* WARNING C5919: Conversion of floating to unsigned integral */
#pragma MESSAGE DISABLE C4002          /* Disable warning C4002 "Result not used" */

static bool EnMode;                    /* Enable/Disable device in a given speed CPU mode */
static bool EnUser;                    /* Enable device by user */
static word CmpHighVal;                /* Compare register value for high speed CPU mode */
/*
** ===================================================================
**     Method      :  HWEnDi (bean TimerInt)
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
  if (EnUser && EnMode) {
    TPM2SC |= 0x08;                    /* Run counter (set CLKSB:CLKSA) */
  } else {
    TPM2SC &= 0x67;                    /* Stop counter (CLKSB:CLKSA = 00) */
    TPM2CNTH = 0x00;                   /* Clear counter register */
  }
}

/*
** ===================================================================
**     Method      :  TickTimer_Enable (bean TimerInt)
**
**     Description :
**         Enables the bean - it starts the timer. Events may be
**         generated ("DisableEvent"/"EnableEvent").
**     Parameters  : None
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
** ===================================================================
*/
byte TickTimer_Enable(void)
{
  if (!EnMode)                         /* Is the device disabled in the actual speed CPU mode? */
    return ERR_SPEED;                  /* If yes then error */
  if (!EnUser) {                       /* Is the device disabled by user? */
    EnUser = TRUE;                     /* If yes then set the flag "device enabled" */
    HWEnDi();                          /* Enable the device */
  }
  return ERR_OK;                       /* OK */
}

/*
** ===================================================================
**     Method      :  TickTimer_SetFreqHz (bean TimerInt)
**
**     Description :
**         This method sets the new frequency of the generated
**         events. The frequency is expressed in Hz as a 16-bit
**         unsigned integer number.
**         This method is available only if the runtime setting type
**         'from interval' is selected in the Timing dialog box in
**         the Runtime setting area.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Freq            - Frequency to set [in Hz]
**                      (100 to 65535 Hz)
**     Returns     :
**         ---             - Error code, possible codes:
**                           ERR_OK - OK
**                           ERR_SPEED - This device does not work in
**                           the active speed mode
**                           ERR_MATH - Overflow during evaluation
**                           ERR_RANGE - Parameter out of range
** ===================================================================
*/
byte TickTimer_SetFreqHz(word Freq)
{
  dlong rtval;                         /* Result of two 32-bit numbers division */
  word rtword;                         /* Result of 64-bit number division */

  if (!EnMode)                         /* Is the device disabled in the actual speed CPU mode? */
    return ERR_SPEED;                  /* If yes then error */
  if (Freq < 0x64)                     /* Is the given value out of range? */
    return ERR_RANGE;                  /* If yes then error */
  rtval[1] = 0x4C46C925 / (dword)Freq; /* Divide high speed CPU mode coefficient by the given value */
  rtval[0] = 0x00;                     /* Convert result to the type dlong */
  if (PE_Timer_LngHi1(rtval[0],rtval[1],&rtword)) /* Is the result greater or equal than 65536 ? */
    rtword = 0xFFFF;                   /* If yes then use maximal possible value */
  CmpHighVal = rtword;                 /* Store result (compare register value for high speed CPU mode) to the variable CmpHighVal */
  TickTimer_SetCV(CmpHighVal);         /* Store appropriate value to the compare register according to the selected high speed CPU mode */
  return ERR_OK;                       /* OK */
}

/*
** ===================================================================
**     Method      :  TickTimer_SetHigh (bean TimerInt)
**
**     Description :
**         The method reconfigures the bean and its selected peripheral(s)
**         when the CPU is switched to the High speed mode. The method is 
**         called automatically as s part of the CPU SetHighSpeed method.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void TickTimer_SetHigh(void)
{
  TickTimer_SetCV(CmpHighVal);         /* Store appropriate value to the compare register according to the value of mode identifier if selected High speed CPU mode */
  TickTimer_SetPV(0x0A);               /* Set prescaler */
  EnMode = TRUE;                       /* Set the flag "device enabled" in the actual speed CPU mode */
  HWEnDi();
}

/*
** ===================================================================
**     Method      :  TickTimer_SetLow (bean TimerInt)
**
**     Description :
**         The method reconfigures the bean and its selected peripheral(s)
**         when the CPU is switched to the Low speed mode. The method is 
**         called automatically as a part of the CPU SetLowSpeed method.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void TickTimer_SetLow(void)
{
  EnMode = FALSE;                      /* Set the flag "device disabled" in the actual speed CPU mode */
  HWEnDi();
}

/*
** ===================================================================
**     Method      :  TickTimer_Init (bean TimerInt)
**
**     Description :
**         Initializes the associated peripheral(s) and the beans 
**         internal variables. The method is called automatically as a 
**         part of the application initialization code.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
void TickTimer_Init(void)
{
  CmpHighVal=0x1386;                   /* Remember appropriate value of compare register for high speed CPU mode */
  EnUser=FALSE;                        /* Disable device */
  /* TPM2SC: TOF=0,TOIE=0,CPWMS=0,CLKSB=0,CLKSA=0,PS2=0,PS1=0,PS0=0 */
  setReg8(TPM2SC, 0x00);                
  /* TPM2C1SC: CH1F=0,CH1IE=1,MS1B=0,MS1A=1,ELS1B=0,ELS1A=0,??=0,??=0 */
  setReg8(TPM2C1SC, 0x50);              
  TickTimer_SetHigh();                 /* Initial speed CPU mode is high */
}

/*
** ===================================================================
**     Method      :  TickTimer_Interrupt (bean TimerInt)
**
**     Description :
**         The method services the interrupt of the selected peripheral(s)
**         and eventually invokes the beans event(s).
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
ISR(TickTimer_Interrupt)
{
  TPM2C1SC_CH1F = 0;                   /* Reset compare interrupt request flag */
  vEventTickInterrupt();               /* Invoke user event */
}

/*
** ===================================================================
**     Method      :  SetCV (bean TimerInt)
**
**     Description :
**         The method computes and sets compare eventually modulo value 
**         for time measuring.
**         This method is internal. It is used by Processor Expert only.
** ===================================================================
*/
static void TickTimer_SetCV(word Val)
{
  TPM2C1V = Val;                       /* Store given value to the compare register */
  TPM2MOD = Val;                       /* and to the modulo register */
}

/* END TickTimer. */

/*
** ###################################################################
**
**     This file was created by UNIS Processor Expert 2.97 [03.74]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/