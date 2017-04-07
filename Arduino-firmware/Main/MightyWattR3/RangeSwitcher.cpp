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
#include "Communication.h"
//#include "Measurement.h"
//#include "CurrentSetter.h"
//#include "VoltageSetter.h"

/* </Includes> */ 


/* <Module variables> */ 

static RangeSwitcher_CurrentRanges currentRange = CURRENT_DEFAULT_HARDWARE_RANGE;
static RangeSwitcher_VoltageRanges voltageRange = VOLTAGE_DEFAULT_HARDWARE_RANGE;
static bool currentRangeAuto, voltageRangeAuto; /* Defines whether the load can use autoranging by voltage setter and current setter. If true, it can, if false, the range will be fixed on high range */
static const Communication_WriteCommand * writeCommand; /* Pointer to the write command where new data from communication can be found */
static uint8_t commandCounter; /* Number of the last executed command from communication */

/* </Module variables> */ 


/* <Implementations> */ 

void RangeSwitcher_Init(void)
{
  pinMode(CURRENT_GAIN_PIN, OUTPUT);
  pinMode(VOLTAGE_GAIN_PIN, OUTPUT);
  RangeSwitcher_SetCurrentRange(CURRENT_DEFAULT_HARDWARE_RANGE);
  RangeSwitcher_SetVoltageRange(VOLTAGE_DEFAULT_HARDWARE_RANGE);
  currentRangeAuto = true;
  voltageRangeAuto = true;
  writeCommand = Communication_GetWriteCommand();
  commandCounter = 0;
}

void RangeSwitcher_Do(void)
{
  /* Check new command */
  if (writeCommand->commandCounter != commandCounter)
  {
    /* LSB first */
    switch (writeCommand->command)
    {
      case WriteCommand_CurrentRangeAuto:
        currentRangeAuto = ((writeCommand->data)[0]) > 0;
      break;
      case WriteCommand_VoltageRangeAuto:
        voltageRangeAuto = ((writeCommand->data)[0]) > 0;
      break;
      default:
      /* command handled by other modules */
      break;
    }
    commandCounter = writeCommand->commandCounter;
  }  
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

bool RangeSwitcher_CanAutorangeCurrent(void)
{
  return currentRangeAuto;
}

bool RangeSwitcher_CanAutorangeVoltage(void)
{
  return voltageRangeAuto;
}

/* </Implementations> */ 

