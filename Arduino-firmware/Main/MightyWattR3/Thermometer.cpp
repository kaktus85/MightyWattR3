/**
 * Thermometer.cpp
 *
 * 2016-10-23
 * kaktus circuits
 * GNU GPL v.3
 */
 
 
 /* <Includes> */ 

#include "Arduino.h"
#include "Thermometer.h"
#include "ADC.h"
#include "DACC.h"
#include "Measurement.h"

/* </Includes> */ 


/* <Module variables> */ 

static TSCUChar temperature; /* struct containing temperature in Celsius */
static const TSCADCLong * ADCRaw;
static uint8_t adcCounter;
static ErrorMessaging_Error thermometerError;

/* </Module variables> */ 


/* <Implementations> */ 

void Thermometer_Init(void)
{ 
  ADC_SetupChannel(ADC_T, Measurement_Speed[Measurement_Fast]);
  ADCRaw = ADC_GetVoltage(ADC_T);
  adcCounter = ADCRaw->counter;
  temperature.counter = 0;
  temperature.value = 0;
  temperature.milliseconds = 0;
  thermometerError.errorCounter = 0;
  thermometerError.error = ErrorMessaging_Thermometer_HardwareFault;
}

void Thermometer_Do(void)
{   
  if (adcCounter != ADCRaw->counter) /* Process only new reading from ADC */
  {  
    float rawTemperature;
    int32_t thermistorResistance; 
    if ((ADCRaw->value <= 0) || (THERMOMETER_REFERENCE_VOLTAGE_IN_ADC_LSB <= ADCRaw->value))
    {
      /* ERROR */
      thermometerError.errorCounter++;
      thermometerError.error = ErrorMessaging_Thermometer_HardwareFault;
      return;
    }
    /* calculate resistance of the thermistor */
    thermistorResistance = (THERMISTOR_R * (int64_t)(ADCRaw->value)) / THERMOMETER_REFERENCE_VOLTAGE_IN_ADC_LSB;
    if (thermistorResistance <= 0)
    {
      /* ERROR */      
      thermometerError.errorCounter++;
      thermometerError.error = ErrorMessaging_Thermometer_HardwareFault;
      return;
    }
    
    rawTemperature = 1 / (THERMISTOR_EQ_A + THERMISTOR_EQ_B * log((float)thermistorResistance)) - 273.15;        
    
    if (rawTemperature < 0)
    {
      rawTemperature = 0;
    }
    else if (rawTemperature > 255)
    {
      rawTemperature = 255;
    }
    
    temperature.value = (uint8_t)rawTemperature;    
    adcCounter = ADCRaw->counter;
    temperature.counter++;
    temperature.milliseconds = millis();
  }
}

const TSCUChar * Thermometer_GetTemperature(void)
{
  return &temperature;
}

const ErrorMessaging_Error * Thermometer_GetError(void)
{
  return &thermometerError;
}

/* </Implementations> */ 
