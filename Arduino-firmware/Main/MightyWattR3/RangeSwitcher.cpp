/**
 * RangeSwitcher.cpp
 * manages current and voltage range switching
 *
 * 2017-03-25
 * kaktus circuits
 * GNU GPL v.3
 */
 
 
/* <Includes> */ 

#include "Arduino.h"
#include "RangeSwitcher.h"
//#include "Measurement.h"
//#include "CurrentSetter.h"
//#include "VoltageSetter.h"

/* </Includes> */ 


/* <Module variables> */ 

static RangeSwitcher_CurrentRanges currentRange = CURRENT_DEFAULT_HARDWARE_RANGE;
static RangeSwitcher_VoltageRanges voltageRange = VOLTAGE_DEFAULT_HARDWARE_RANGE;

/* </Module variables> */ 


/* <Implementations> */ 

void RangeSwitcher_Init(void)
{
  pinMode(CURRENT_GAIN_PIN, OUTPUT);
  pinMode(VOLTAGE_GAIN_PIN, OUTPUT);
  RangeSwitcher_SetCurrentRange(CURRENT_DEFAULT_HARDWARE_RANGE);
  RangeSwitcher_SetVoltageRange(VOLTAGE_DEFAULT_HARDWARE_RANGE);
}

void RangeSwitcher_SetCurrentRange(RangeSwitcher_CurrentRanges range)
{  
  currentRange = range;
  
  switch (currentRange)
  {
    case CurrentRange_LowCurrent:
      digitalWrite(CURRENT_GAIN_PIN, HIGH);
    break;
    case CurrentRange_HighCurrent:
      digitalWrite(CURRENT_GAIN_PIN, LOW);
    break;
    default:
    break;
  }
}

void RangeSwitcher_SetVoltageRange(RangeSwitcher_VoltageRanges range)
{  
  voltageRange = range;

  switch (voltageRange)
  {
    case VoltageRange_LowVoltage:
      digitalWrite(VOLTAGE_GAIN_PIN, HIGH);
    break;
    case VoltageRange_HighVoltage:
      digitalWrite(VOLTAGE_GAIN_PIN, LOW);
    break;
    default:
    break;
  }
}

RangeSwitcher_CurrentRanges RangeSwitcher_GetCurrentRange(void)
{
  return currentRange;
}

RangeSwitcher_VoltageRanges RangeSwitcher_GetVoltageRange(void)
{
  return voltageRange;
}

/* </Implementations> */ 

