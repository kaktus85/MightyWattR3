/**
 * DACC.cpp
 * DACC management
 *
 * 2016-10-23
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "DACC.h"

/* </Includes> */ 


/* <Module variables> */ 

static uint16_t dacValue;
static ErrorMessaging_Error dacError;
const static ErrorMessaging_Error * AD569xRError;

/* </Module variables> */ 


/* <Implementations> */ 

void DACC_Init(void)
{
  AD569xR_Init();
  dacValue = 0;
  dacError.errorCounter = 0;
  dacError.error = ErrorMessaging_DACC_UpperLimitReached;
  AD569xRError = AD569xR_GetError();
}

bool DACC_SetVoltage(uint16_t value)
{        
  if (dacValue != value) /* Only update value if different from previous value */
  {
    if (AD569xR_Set(value)) /* check command success */
    {
      dacValue = value;
    }
    else
    {
      dacError.errorCounter++;
      dacError.error = AD569xRError->error;
      return false;
    }     
  }
  
  return true;
}

bool DACC_SetPercentOfRange(uint8_t percentage)
{  
  if (percentage >= 100)
  {
    return DACC_SetVoltage(DAC_MAXIMUM);
  }
  else
  {
    return DACC_SetVoltage((((uint32_t)DAC_MAXIMUM) * ((uint32_t)percentage)) / 100);
  }
}

bool DACC_Plus(uint16_t value)
{
  bool result = false;
  if ((uint32_t)value + (uint32_t)dacValue <= DAC_MAXIMUM)
  {
    result = DACC_SetVoltage(dacValue + value);
  }
  else if (DACC_SetVoltage(DAC_MAXIMUM))
  {
    dacError.errorCounter++;
    dacError.error = ErrorMessaging_DACC_UpperLimitReached;
  }
  return result;
}

bool DACC_Minus(uint16_t value)
{ 
  bool result = false;
  if (dacValue >= value)
  {
    result = DACC_SetVoltage(dacValue - value);
  }
  else if (DACC_SetVoltage(0))
  {
    dacError.errorCounter++;
    dacError.error = ErrorMessaging_DACC_LowerLimitReached;
  }
  return result;
}

uint16_t DACC_GetValue()
{
  return dacValue;
}

const ErrorMessaging_Error * DACC_GetError(void)
{
  return &dacError;
}

/* </Implementations> */ 
