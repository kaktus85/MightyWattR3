/**
 * CurrentSetter.cpp
 *
 * 2016-10-29
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Control.h"
#include "DACC.h"
#include "Ammeter.h"
#include "CurrentSetter.h"
#include "Configuration.h"
#include "RangeSwitcher.h"

/* </Includes> */ 


/* <Module variables> */ 

static ErrorMessaging_Error CurrentSetterError;
static const uint16_t minDACHi = (((uint64_t)(CURRENTSETTER_OFFSET_HI)) << 16) / CURRENTSETTER_SLOPE_HI;
static const uint16_t minDACLo = (((uint64_t)(CURRENTSETTER_OFFSET_HI)) << 16) / CURRENTSETTER_SLOPE_HI;
static uint32_t presentCurrent;

/* </Module variables> */ 


/* <Implementations> */ 

void CurrentSetter_Init(void)
{
  CurrentSetterError.errorCounter = 0;
  CurrentSetterError.error = ErrorMessaging_CurrentSetter_SetCurrentOverload;
}

void CurrentSetter_Do(void)
{
  uint32_t dac;
  
  if (presentCurrent > 0) /* on zero current set true zero to DAC */
  {      
    switch (RangeSwitcher_GetCurrentRange())
    {
      case CurrentRange_HighCurrent:        
        dac = (((uint64_t)((int32_t)presentCurrent + CURRENTSETTER_OFFSET_HI)) << 16) / CURRENTSETTER_SLOPE_HI;
        if (dac > DAC_MAXIMUM) /* Set current higher than maximum */
        {
          CurrentSetterError.errorCounter++;
          CurrentSetterError.error = ErrorMessaging_CurrentSetter_SetCurrentOverload;
          dac = DAC_MAXIMUM;
        }      
      break;
      case CurrentRange_LowCurrent:
        dac = (((uint64_t)((int32_t)presentCurrent + CURRENTSETTER_OFFSET_LO)) << 16) / CURRENTSETTER_SLOPE_LO;
        if (dac > DAC_MAXIMUM)
        {
          RangeSwitcher_CurrentRangeUp(); // increase range and call this function recursively
          CurrentSetter_Do();
          return;
        }        
      break;
      default:      
      return;
    }
  }
  else
  {
    dac = 0;
  }
    
  if (!DACC_SetVoltage(dac & 0xFFFF)) /* Set calculated current */
  {
    CurrentSetterError.errorCounter++;
    CurrentSetterError.error = ErrorMessaging_CurrentSetter_SetCurrentOverload; 
  }
  Control_SetCCCV(Control_CCCV_CC); /* Set phase CC */
}

void CurrentSetter_SetCurrent(uint32_t current)
{
  presentCurrent = current;
}

void CurrentSetter_SetZero()
{
  presentCurrent = 0;
  Control_SetCCCV(Control_CCCV_CC); /* Set phase CC */  
  DACC_SetVoltage(0);
}

//bool CurrentSetter_SetCurrent(uint32_t current)
//{  
//  uint32_t dac;
//  bool result = true;
//  presentCurrent = current;
//  
//  if (current > 0) /* on zero current set true zero to DAC */
//  {      
//    switch (RangeSwitcher_GetCurrentRange())
//    {
//      case CurrentRange_HighCurrent:
//        if ((int32_t)current > 0)
//        {
//          dac = (((uint64_t)((int32_t)current + CURRENTSETTER_OFFSET_HI)) << 16) / CURRENTSETTER_SLOPE_HI;
//          if (dac > DAC_MAXIMUM) /* Set current higher than maximum */
//          {
//            CurrentSetterError.errorCounter++;
//            CurrentSetterError.error = ErrorMessaging_CurrentSetter_SetCurrentOverload;
//            dac = DAC_MAXIMUM;
//            result = false;
//          }
//        }
//        else
//        {
//          dac = 0;
//        }
//      break;
//      case CurrentRange_LowCurrent:
//        if ((int32_t)current > 0)
//        {
//          dac = (((uint64_t)((int32_t)current + CURRENTSETTER_OFFSET_LO)) << 16) / CURRENTSETTER_SLOPE_LO;
//          if (dac > DAC_MAXIMUM) /* Set maximum DAC at low range and wait for automatic range switch */
//          {
//            RangeSwitcher_CurrentRangeUp(); // increase range and call this function recursively
//            return CurrentSetter_SetCurrent(current);
//          }
//        }
//        else
//        {
//          dac = 0;
//        }
//      break;
//      default:      
//      return false;
//    }
//  }
//  else
//  {
//    dac = 0;
//  }
//  
//  Control_SetCCCV(Control_CCCV_CC); /* Set phase CC */
//  if (!DACC_SetVoltage(dac & 0xFFFF)) /* Set calculated current */
//  {
//    CurrentSetterError.errorCounter++;
//    CurrentSetterError.error = ErrorMessaging_CurrentSetter_SetCurrentOverload;
//    result = false;    
//  }
//  
//  return result;
//}

void CurrentSetter_Plus(uint32_t value)
{
  if ((presentCurrent + value) < presentCurrent || (presentCurrent + value) < value) // overflow check
  {  
    CurrentSetter_SetCurrent(CURRENT_SETTER_MAXIMUM_HICURRENT);
  }
  else
  {
    CurrentSetter_SetCurrent(value + presentCurrent);
  }
}

void CurrentSetter_Minus(uint32_t value)
{
  if (value > presentCurrent) // overflow check
  {  
    CurrentSetter_SetCurrent(0);
  }
  else
  {
    CurrentSetter_SetCurrent(presentCurrent - value); 
  }
}
//
//void CurrentSetter_Plus(uint16_t value)
//{
//  Control_SetCCCV(Control_CCCV_CC); /* Set phase CC */  
//
//  uint16_t minDAC = 0;
//  uint16_t presentDAC = DACC_GetValue();
//  // calculate minimum DAC value due to DAC offset
//  switch (RangeSwitcher_GetCurrentRange())
//  {
//    case CurrentRange_HighCurrent:
//      minDAC = minDACHi;
//    break;
//    case CurrentRange_LowCurrent:
//      minDAC = minDACLo;      
//    break;
//    default:
//    break;
//  }
//
//  if ((presentDAC + value) < presentDAC || (presentDAC + value) < value) // overflow check
//  {
//    DACC_Plus(value);
//  }  
//  else
//  {
//    if (presentDAC + value < minDAC)
//    {
//      DACC_SetVoltage(minDAC); // set minimum DAC value - start from electrical zero, not DAC logical zero
//    }
//    else
//    {
//      DACC_Plus(value);
//    }
//  }
//}
//
//void CurrentSetter_Minus(uint16_t value)
//{
//  Control_SetCCCV(Control_CCCV_CC); /* Set phase CC */
//  
//  uint16_t minDAC = 0;;
//  uint16_t presentDAC = DACC_GetValue();
//  // calculate minimum DAC value due to DAC offset
//  switch (RangeSwitcher_GetCurrentRange())
//  {
//    case CurrentRange_HighCurrent:
//      minDAC = minDACHi;
//    break;
//    case CurrentRange_LowCurrent:
//      minDAC = minDACLo;      
//    break;
//    default:
//    break;
//  }
//
//  if (presentDAC < value) // overflow check
//  {
//    DACC_SetVoltage(0); // set zero to DAC
//  }  
//  else
//  {
//    if (presentDAC - value < minDAC)
//    {
//      DACC_SetVoltage(0); // set zero to DAC
//    }
//    else
//    {
//      DACC_Minus(value);
//    }
//  }
//}

uint32_t CurrentSetter_GetCurrent(void)
{
  return presentCurrent;
}

const ErrorMessaging_Error * CurrentSetter_GetError(void)
{
  return &CurrentSetterError;
}

/* </Implementations> */ 
