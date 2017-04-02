/**
 * VoltageSetter.cpp
 *
 * 2016-10-29
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "Control.h"
#include "DACC.h"
#include "Voltmeter.h"
#include "VoltageSetter.h"
#include "RangeSwitcher.h"

/* </Includes> */ 


/* <Module variables> */ 

//static uint16_t dacTargetValue;
static ErrorMessaging_Error VoltageSetterError;
static uint32_t presentVoltage;

/* </Module variables> */ 


/* <Implementations> */ 

void VoltageSetter_Init(void)
{
  VoltageSetterError.errorCounter = 0;
  VoltageSetterError.error = ErrorMessaging_VoltageSetter_SetVoltageOverload;
}

void VoltageSetter_Do(void)
{
  uint32_t dac;
  RangeSwitcher_VoltageRanges range;
  
  /* Calculate range */
  if (presentVoltage > VOLTAGESETTER_HYSTERESIS_UP)
  {
    range = VoltageRange_HighVoltage;
  }
  else if (presentVoltage < VOLTAGESETTER_HYSTERESIS_DOWN)
  {
    range = VoltageRange_LowVoltage;
  }
  else
  {
    range = RangeSwitcher_GetVoltageRange();
  }
      
  switch (range)
  {
    case VoltageRange_HighVoltage:
      dac = (((uint64_t)((int32_t)presentVoltage + VOLTSETTER_OFFSET_HI)) << 16) / VOLTSETTER_SLOPE_HI;
      if (dac > DAC_MAXIMUM) /* Set voltage higher than maximum */
      {
        VoltageSetterError.errorCounter++;
        VoltageSetterError.error = ErrorMessaging_VoltageSetter_SetVoltageOverload;
        dac = DAC_MAXIMUM;
      }
    break;
    case VoltageRange_LowVoltage:
      dac = (((uint64_t)((int32_t)presentVoltage + VOLTSETTER_OFFSET_LO)) << 16) / VOLTSETTER_SLOPE_LO;
    break;
    default:      
    return;
  }

  /* Set range if high voltage */
  if (range == VoltageRange_HighVoltage)
  {
    RangeSwitcher_SetVoltageRange(range);
  }
  /* Set calculated DAC value */
  if (!DACC_SetVoltage(dac & 0xFFFF))
  {
    VoltageSetterError.errorCounter++;
    VoltageSetterError.error = ErrorMessaging_VoltageSetter_SetVoltageOverload; 
  }  
  /* Set range if low voltage */
  if (range == VoltageRange_LowVoltage)
  {
    RangeSwitcher_SetVoltageRange(range);
  }
  /* Set phase CV */
  Control_SetCCCV(Control_CCCV_CV);
}

void VoltageSetter_SetVoltage(uint32_t voltage)
{
  presentVoltage = voltage;
}

uint32_t VoltageSetter_GetVoltage(void)
{
  return presentVoltage;
}

void VoltageSetter_Plus(uint32_t value)
{
  if ((presentVoltage + value) < presentVoltage || (presentVoltage + value) < value) // overflow check
  {  
    VoltageSetter_SetVoltage(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE);
  }
  else
  {
    VoltageSetter_SetVoltage(value + presentVoltage);
  }
}

void VoltageSetter_Minus(uint32_t value)
{
  if (value > presentVoltage) // overflow check
  {  
    VoltageSetter_SetVoltage(0);
  }
  else
  {
    VoltageSetter_SetVoltage(presentVoltage - value); 
  }
}

const ErrorMessaging_Error * VoltageSetter_GetError(void)
{
  return &VoltageSetterError;
}

/* </Implementations> */ 
