/**
 * Measurement.cpp
 *
 * 2016-10-22
 * kaktus circuits
 * GNU GPL v.3
 */

/* <Includes> */ 

#include "Arduino.h"
#include "Measurement.h"
#include "Ammeter.h"
#include "Voltmeter.h"
#include "Configuration.h"
#include "Communication.h"

/* </Includes> */ 
 
 
/* <Module variables> */ 

static const Communication_WriteCommand * writeCommand; /* Pointer to the write command where new data from communication can be found */
static const TSCADCULong * voltage;
static const TSCADCULong * current;
static uint8_t voltageCounter, currentCounter, voltageErrorCounter, currentErrorCounter;
static Measurement_Values measurementValues;
static ErrorMessaging_Error MeasurementError;
static uint8_t commandCounter;
static bool invalidated; /* Indicates that the next measurement will be considered invalid */

#ifdef ADC_TYPE_ADS1015
const ADC_RateRangingFilter MeasurementFast = {ADS1015_920SPS, false, false};
const ADC_RateRangingFilter MeasurementMedium = {ADS1015_920SPS, true, false};
const ADC_RateRangingFilter MeasurementSlow = {ADS1015_920SPS, true, true};
#elif defined(ADC_TYPE_ADS1115)
const ADC_RateRangingFilter MeasurementFast = {ADS1115_860SPS, false, false};
const ADC_RateRangingFilter MeasurementMedium = {ADS1115_860SPS, true, false};
const ADC_RateRangingFilter MeasurementSlow = {ADS1115_860SPS, true, true};
#else
#error No ADC defined
#endif
 
const ADC_RateRangingFilter Measurement_Speed[] = {MeasurementFast, MeasurementMedium, MeasurementSlow};
static const ErrorMessaging_Error * AmmeterError;
static const ErrorMessaging_Error * VoltmeterError;

/* </Module variables> */ 


/* <Implementations> */ 

void Measurement_Init(void)
{
  voltage = Voltmeter_GetVoltage();
  current = Ammeter_GetCurrent();
  voltageCounter = 0;
  currentCounter = 0;
  measurementValues.counter = 0;
  measurementValues.milliseconds = 0;
  measurementValues.voltage = 0;
  measurementValues.current = 0;
  measurementValues.power = 0;
  measurementValues.resistance = VOLTMETER_INPUT_RESISTANCE;
  measurementValues.unfilteredVoltage = 0;
  measurementValues.unfilteredCurrent = 0;
  measurementValues.unfilteredPower = 0;
  measurementValues.unfilteredResistance = VOLTMETER_INPUT_RESISTANCE; 
  
  AmmeterError = Ammeter_GetError();
  VoltmeterError = Voltmeter_GetError();
  currentErrorCounter = AmmeterError->errorCounter;
  voltageErrorCounter = VoltmeterError->errorCounter;
  
  MeasurementError.error = ErrorMessaging_Measurement_Invalid;
  MeasurementError.errorCounter = 0;
  
  writeCommand = Communication_GetWriteCommand();
  commandCounter = writeCommand->commandCounter;

  invalidated = false;
}

void Measurement_Do(void)
{  
  /* Check new command */
  if (writeCommand->commandCounter != commandCounter)
  {
    /* LSB first */
    switch (writeCommand->command)
    {
      case WriteCommand_MeasurementSpeed:
      {
        uint8_t newSpeed = (writeCommand->data)[0];
        if (newSpeed < MEASUREMENT_SPEEDS_COUNT)
        {
          Ammeter_SetSpeed((Measurement_Speeds)newSpeed);
          Voltmeter_SetSpeed((Measurement_Speeds)newSpeed);
        }
        break;
      }
      default:
      /* command handled by other modules */
      break;
    }
    commandCounter = writeCommand->commandCounter;
  }

  if ((voltageCounter != voltage->counter) && (currentCounter != current->counter)) /* Calculate values when both voltage and current are updated */
  {       
    voltageCounter = voltage->counter;
    currentCounter = current->counter;
    
    if (invalidated)
    {
      invalidated = false;
    }
    else
    {    
      uint64_t resistance, unfilteredResistance = 0;
      measurementValues.voltage = voltage->value;
      measurementValues.current = current->value;
      measurementValues.power = (uint32_t)((((uint64_t)measurementValues.voltage) * ((uint64_t)measurementValues.current)) / 1000000ULL);
      measurementValues.unfilteredVoltage = voltage->unfilteredValue;
      measurementValues.unfilteredCurrent = current->unfilteredValue;
      measurementValues.unfilteredPower = (uint32_t)((((uint64_t)measurementValues.unfilteredVoltage) * ((uint64_t)measurementValues.unfilteredCurrent)) / 1000000ULL);
      if (measurementValues.current == 0) /* Zero current implies maximum input resistance, which is determined by voltmeter input resistance */
      {
        resistance = VOLTMETER_INPUT_RESISTANCE;
      }
      else
      {
        resistance = (((((uint64_t)measurementValues.voltage) << 22) * 1000) / ((uint64_t)measurementValues.current)) >> 22;
        if (resistance > ((uint64_t)VOLTMETER_INPUT_RESISTANCE))
        {
          resistance = VOLTMETER_INPUT_RESISTANCE; /* Resistance cannot be larger than the voltmeter input resistance */
        }
        if (resistance > ((uint64_t)VOLTMETER_INPUT_RESISTANCE))
        {
          resistance = VOLTMETER_INPUT_RESISTANCE; /* Resistance cannot be larger than the voltmeter input resistance */
        }
      }
      measurementValues.resistance = (uint32_t)resistance;
  
      if (measurementValues.unfilteredCurrent == 0) /* Zero current implies maximum input resistance, which is determined by voltmeter input resistance */
      {
        unfilteredResistance = VOLTMETER_INPUT_RESISTANCE;
      }
      else
      {
        unfilteredResistance = (((((uint64_t)measurementValues.unfilteredVoltage) << 22) * 1000) / ((uint64_t)measurementValues.unfilteredCurrent)) >> 22;
        if (unfilteredResistance > ((uint64_t)VOLTMETER_INPUT_RESISTANCE))
        {
          unfilteredResistance = VOLTMETER_INPUT_RESISTANCE; /* Resistance cannot be larger than the voltmeter input resistance */
        }
        if (unfilteredResistance > ((uint64_t)VOLTMETER_INPUT_RESISTANCE))
        {
          unfilteredResistance = VOLTMETER_INPUT_RESISTANCE; /* Resistance cannot be larger than the voltmeter input resistance */
        }
      }
      measurementValues.unfilteredResistance = (uint32_t)unfilteredResistance;             
      measurementValues.counter++;
      measurementValues.milliseconds = millis();
        
      if ((currentErrorCounter != AmmeterError->errorCounter) || (voltageErrorCounter != VoltmeterError->errorCounter))
      {
        /* Measurement invalid */
        MeasurementError.error = ErrorMessaging_Measurement_Invalid;
        MeasurementError.errorCounter++;
        currentErrorCounter = AmmeterError->errorCounter;
        voltageErrorCounter = VoltmeterError->errorCounter;
      }
    }
  }
}

const Measurement_Values * Measurement_GetValues(void)
{
  return &measurementValues;
}

void Measurement_Invalidate(void)
{
  invalidated = true;
}

const ErrorMessaging_Error * Measurement_GetError(void)
{
  return &MeasurementError;
}

/* </Implementations> */ 
