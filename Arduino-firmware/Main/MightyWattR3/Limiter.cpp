/**
 * Limiter.cpp
 *
 * 2016-11-02
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "Thermometer.h"
#include "Measurement.h"
#include "Communication.h"
#include "Configuration.h"
#include "Control.h"
#include "Limiter.h"
#include "Voltmeter.h"
#include "Ammeter.h"
#include "VoltageSetter.h"
#include "CurrentSetter.h"
#include "Integrator.h"

/* </Includes> */ 


/* <Module variables> */ 

static const Communication_WriteCommand * writeCommand; /* Pointer to the write command where new data from communication can be found */
static const Measurement_Values * measurementValues; /* Pointer to the latest measured voltage, current, power and resistance */
static const TSCUChar * temperature; /* Pointer to structure where temperature can be found */
static uint8_t commandCounter, temperatureCounter; /* Number of the last executed command from communication, number of the last temperature data */
static uint8_t measurementErrorCounter, thermometerErrorCounter, ADCErrorCounter[ADC_CHANNEL_COUNT]; /* Error counters for measurement, thermometer and ADC modules */
static uint16_t SeriesResistance; /* Series resistance for calculating allowed P in 4-wire mode, in mOhm (max 65.535 Ohm) */
const static ErrorMessaging_Error * MeasurementError; /* Pointer to error structure from measurement */
const static ErrorMessaging_Error * ThermometerError; /* Pointer to error structure from thermometer */
const static ErrorMessaging_Error * ADCError[ADC_CHANNEL_COUNT]; /* Pointer to error structure from ADC */
static ErrorMessaging_Error LimiterError; /* This module error message */

static Integrator_Limits voltageLimits, currentLimits, powerLimits;

/* </Module variables> */ 


/* <Declarations (prototypes)> */ 

/**
 * Checks limits and takes action (stops load)
 */
static void Limiter_Keep(void);

/* </Declarations (prototypes)> */ 


/* <Implementations> */ 

void Limiter_Init(void)
{
  uint8_t i;
  
  writeCommand = Communication_GetWriteCommand();  
  measurementValues = Measurement_GetValues();
  temperature = Thermometer_GetTemperature();
  
  SeriesResistance = 0;
  
  voltageLimits.limit = VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE / 100000; /* 100 mV */
  voltageLimits.allowedOverload = LIMITER_VOLTAGE_ALLOWED_OVERLOAD; /* 100 mV.ms*/
  currentLimits.limit = CURRENT_SETTER_MAXIMUM_HICURRENT / 100000; /* 100 mA */
  currentLimits.allowedOverload = LIMITER_CURRENT_ALLOWED_OVERLOAD; /* 100 mA.ms*/  
  powerLimits.allowedOverload = LIMITER_POWER_ALLOWED_OVERLOAD;  
  
  MeasurementError = Measurement_GetError();
  measurementErrorCounter = MeasurementError->errorCounter;
  ThermometerError = Thermometer_GetError();
  thermometerErrorCounter = ThermometerError->errorCounter;
  for (i = 0; i < ADC_CHANNEL_COUNT; i++)
  {
    ADCError[i] = ADC_GetError((ADC_Channels)i);
    ADCErrorCounter[i] = ADCError[i]->errorCounter;
  }
    
  LimiterError.errorCounter = 0;
  LimiterError.error = ErrorMessaging_Measurement_Invalid;
}

void Limiter_Do(void)
{
   /* Check new command */
  if (writeCommand->commandCounter != commandCounter)
  {
    /* LSB first */
    switch (writeCommand->command)
    {
      case WriteCommand_SeriesResistance:
        SeriesResistance = Data_GetUIntFromUCharArray(writeCommand->data);
      break;
      default:
      /* command handled by other modules */
      break;
    }
    commandCounter = writeCommand->commandCounter;
  }  

  Limiter_Keep();  
}

static void Limiter_Keep(void)
{
  uint8_t i;
  bool fatalError = false;
  static Measurement_Values lastValues; /* copy of last measurement values */
  static uint32_t voltageIntegral, currentIntegral, powerIntegral;
  uint32_t maximumPower; /* SOA-limited, mW */
  
  /* Check validity of measured values */
  if (measurementErrorCounter != MeasurementError->errorCounter)
  {
    if (MeasurementError->error == ErrorMessaging_Measurement_Invalid)
    {
      /* Measurement is invalid, the values cannot be assumed correct */
      fatalError = true;
      LimiterError.error = ErrorMessaging_Limiter_HardwareFault;  
    }
    measurementErrorCounter = MeasurementError->errorCounter;    
  }
  if (thermometerErrorCounter != ThermometerError->errorCounter)
  {
    if (ThermometerError->error == ErrorMessaging_Thermometer_HardwareFault)
    {
      /* Thermometer hardware is faulty, temperature cannot be assumed correct */
      fatalError = true;
      LimiterError.error = ErrorMessaging_Limiter_HardwareFault;  
    }
    thermometerErrorCounter = ThermometerError->errorCounter;    
  }
  for (i = 0; i < ADC_CHANNEL_COUNT; i++)
  {    
    if (ADCErrorCounter[i] != ADCError[i]->errorCounter)
    {
      if (ADCError[i]->error == ErrorMessaging_ADC_NotResponding)
      {
        /* ADC does not respond, no new values can be measured and the load is in unknown state */
        fatalError = true;
        LimiterError.error = ErrorMessaging_Limiter_HardwareFault;  
      }
      ADCErrorCounter[i] = ADCError[i]->errorCounter;    
    }
  }
  
  /* Temperature check */
  if (temperatureCounter != temperature->counter)
  {
    if (temperature->value > LIMITER_MAXIMUM_TEMPERATURE)
    {
      /* Overheat */
      fatalError = true;     
      LimiterError.error = ErrorMessaging_Limiter_Overheat;   
//      Serial.print("Temperature: ");
//      Serial.print(temperature->value);
//      Serial.println(" °C");
    }    
    temperatureCounter = temperature->counter;
  }  
  
  /* Voltage, current and power check */
  if (lastValues.counter != measurementValues->counter)
  { 
    /* V check */
    if (measurementValues->voltage > VOLTMETER_MAXIMUM_VOLTAGE)
    {
      /* Measured voltage out of range - hard limit (ADC) */
      fatalError = true;
      LimiterError.error = ErrorMessaging_Limiter_VoltageOverload;      
    }
    if (Integrator_Add((measurementValues->voltage + 100000) / 200000 + (lastValues.voltage + 100000) / 200000, measurementValues->milliseconds - lastValues.milliseconds, &voltageIntegral, &voltageLimits) == Integrator_Over)
    {    
      /* Measured voltage out of range - soft limit (DAC) */
      fatalError = true;
      LimiterError.error = ErrorMessaging_Limiter_VoltageOverload;
    }   
    
    /* I check */
    if (measurementValues->current > AMMETER_MAXIMUM_CURRENT)
    {
      /* Measured current out of range - hard limit (ADC) */
      fatalError = true;
      LimiterError.error = ErrorMessaging_Limiter_CurrentOverload;
    }
    if (Integrator_Add((measurementValues->current + 100000) / 200000 + (lastValues.current + 100000) / 200000, measurementValues->milliseconds - lastValues.milliseconds, &currentIntegral, &currentLimits) == Integrator_Over)
    {    
      /* Measured current out of range - soft limit (DAC) */
      fatalError = true;
      LimiterError.error = ErrorMessaging_Limiter_CurrentOverload;
    }  
    
    /* P check + SOA limit calculation */
    if (Voltmeter_GetMode() == Voltmeter_4Terminal && (SeriesResistance > 0))
    {
      powerLimits.limit = (MAXIMUM_POWER / 1000 + SeriesResistance * (measurementValues->current / 1000000) * (measurementValues->current / 1000000)) / 100;     
      maximumPower = (LIMITER_MAXIMUM_SOA / 1000) + SeriesResistance * (measurementValues->unfilteredCurrent / 1000000) * (measurementValues->unfilteredCurrent / 1000000) / 1000;
    }
    else
    {      
      powerLimits.limit = MAXIMUM_POWER / 100000;    
      maximumPower = LIMITER_MAXIMUM_SOA / 1000;
    }      
    if (Integrator_Add((measurementValues->power + 100000) / 200000 + (lastValues.power + 100000) / 200000, measurementValues->milliseconds - lastValues.milliseconds, &powerIntegral, &powerLimits) == Integrator_Over)
    {
      /* Measured averaged power out of range */
      fatalError = true;
      LimiterError.error = ErrorMessaging_Limiter_PowerOverload;
    }

    /* SOA check */
    if (measurementValues->unfilteredPower / 1000 > maximumPower)
    {
      /* Measured power above SOA */
      fatalError = true;
      LimiterError.error = ErrorMessaging_Limiter_SOAExceeded;
    }    
        
    lastValues.counter = measurementValues->counter;
    lastValues.milliseconds = measurementValues->milliseconds;
    lastValues.power = measurementValues->power;
    lastValues.voltage = measurementValues->voltage;
    lastValues.current = measurementValues->current;
  }
  
  /* Execute actions */
  if (fatalError)
  {
    Control_StopLoad();
    LimiterError.errorCounter++;
  }
}

const ErrorMessaging_Error * Limiter_GetError(void)
{
  return &LimiterError;
}

/* </Implementations> */ 
