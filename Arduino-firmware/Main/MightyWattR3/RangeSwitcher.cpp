/**
 * RangeSwitcher.cpp
 * manages current and voltage range switching
 *
 * 2017-02-04
 * kaktus circuits
 * GNU GPL v.3
 */
 
 
/* <Includes> */ 

#include "Arduino.h"
#include "RangeSwitcher.h"
#include "Measurement.h"
#include "CurrentSetter.h"
#include "VoltageSetter.h"

/* </Includes> */ 


/* <Declarations (prototypes)> */ 

/**
 * Decides whether there is a need to change the current range and takes the appropriate action
 */
void RangeSwitcher_SetCurrentRange(void);

/**
 * Decides whether there is a need to change the voltage range and takes the appropriate action
 */
void RangeSwitcher_SetVoltageRange(void);

/* </Declarations (prototypes)> */ 


/* <Module variables> */ 

static const Measurement_Values * measurement;
static RangeSwitcher_CurrentRanges currentRange = CURRENT_DEFAULT_HARDWARE_RANGE;
static RangeSwitcher_VoltageRanges voltageRange = VOLTAGE_DEFAULT_HARDWARE_RANGE;
static uint32_t forceCurrentRangeSwitchTimer, forceVoltageRangeSwitchTimer;
static bool canDecreaseCurrentRange = true, canDecreaseVoltageRange = true;

/* </Module variables> */ 


/* <Implementations> */ 

void RangeSwitcher_Init(void)
{
  pinMode(CURRENT_GAIN_PIN, OUTPUT);
  pinMode(VOLTAGE_GAIN_PIN, OUTPUT);
  measurement = Measurement_GetValues();
  RangeSwitcher_SetCurrentRange();
  RangeSwitcher_SetVoltageRange();
}

void RangeSwitcher_Do(void)
{
  RangeSwitcher_SetCurrentRange();  
  RangeSwitcher_SetVoltageRange();
  if ((millis() - forceCurrentRangeSwitchTimer) > FORCE_RANGE_SWITCH_TIMEOUT)
  {
    canDecreaseCurrentRange = true;
  }
  if ((millis() - forceVoltageRangeSwitchTimer) > FORCE_RANGE_SWITCH_TIMEOUT)
  {
    canDecreaseVoltageRange = true;
  }
}

void RangeSwitcher_SetCurrentRange(void)
{  
  if ((CurrentSetter_GetCurrent() > CURRENTSETTER_HYSTERESIS_UP) || (measurement->unfilteredCurrent > AMMETER_HYSTERESIS_UP)) // ADC or DAC over or near limit -> increase current range
  {
    currentRange = CurrentRange_HighCurrent;
  }
  else if ((CurrentSetter_GetCurrent() < CURRENTSETTER_HYSTERESIS_DOWN) && (measurement->unfilteredCurrent < AMMETER_HYSTERESIS_DOWN) && canDecreaseCurrentRange) // Both ADC and DAC below limit -> decrease current range
  {    
    currentRange = CurrentRange_LowCurrent;
  }
  else
  {
    // do nothing
  }

  switch (RangeSwitcher_GetCurrentRange())
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

void RangeSwitcher_SetVoltageRange(void)
{  
  if ((VoltageSetter_GetVoltage() > VOLTAGESETTER_HYSTERESIS_UP) || (measurement->unfilteredVoltage > VOLTMETER_HYSTERESIS_UP)) // ADC or DAC over or near limit -> increase voltage range
  {
    voltageRange = VoltageRange_HighVoltage;
  }
  else if ((VoltageSetter_GetVoltage() < VOLTAGESETTER_HYSTERESIS_DOWN) && (measurement->unfilteredVoltage < VOLTMETER_HYSTERESIS_DOWN) && canDecreaseVoltageRange) // Both ADC and DAC below limit -> decrease voltage range
  {    
    voltageRange = VoltageRange_LowVoltage;
  }
  else
  {
    // do nothing
  }

  switch (RangeSwitcher_GetVoltageRange())
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

void RangeSwitcher_CurrentRangeUp(void)
{
  digitalWrite(CURRENT_GAIN_PIN, LOW);
  currentRange = CurrentRange_HighCurrent;
  forceCurrentRangeSwitchTimer = millis();  
  canDecreaseCurrentRange = false;
}

void RangeSwitcher_VoltageRangeUp(void)
{
  digitalWrite(VOLTAGE_GAIN_PIN, LOW);
  voltageRange = VoltageRange_HighVoltage;
  forceVoltageRangeSwitchTimer = millis();
  canDecreaseVoltageRange = false;
}

/* </Implementations> */ 

