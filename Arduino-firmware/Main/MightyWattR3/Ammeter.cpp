/**
 * Ammeter.cpp
 *
 * 2016-10-22
 * kaktus circuits
 * GNU GPL v.3
 */
 
 
/* <Includes> */ 

#include "Arduino.h"
#include "Ammeter.h"
#include "ADC.h"
#include "Configuration.h"
#include "DACC.h"
#include "RangeSwitcher.h"
#include "Control.h"

/* </Includes> */ 


/* <Module variables> */ 

static const TSCADCLong * ADCRaw;
static TSCADCULong current; /* contains current in microamps */
static uint8_t adcCounter, adcErrorCounter;
static ErrorMessaging_Error AmmeterError;
const static ErrorMessaging_Error * ADCError;
static bool resetFilter;

/* </Module variables> */ 


/* <Implementations> */ 

void Ammeter_Init(void)
{ 
  Ammeter_SetSpeed(AMMETER_DEFAULT_MEASUREMENT_SPEED);
  ADCRaw = ADC_GetVoltage(ADC_I);
  adcCounter = ADCRaw->counter;
  current.value = 0;
  current.unfilteredValue = 0;
  current.counter = 0;
  current.milliseconds = 0;
  AmmeterError.errorCounter = 0;
  AmmeterError.error = ErrorMessaging_Ammeter_CurrentOverload;
  ADCError = ADC_GetError(ADC_I);
  adcErrorCounter = ADCError->errorCounter;
  resetFilter = false;
}

void Ammeter_Do(void)
{
  int32_t signedCurrent, signedUnfilteredCurrent;    
  RangeSwitcher_CurrentRanges range = RangeSwitcher_GetCurrentRange();
  
  if (adcCounter != ADCRaw->counter) /* Process only new reading from ADC */
  {      
    adcCounter = ADCRaw->counter;
        
    if (resetFilter)
    {
      resetFilter = false;
      ADC_ResetFilter(ADC_I); // Reset filter to remove values measured at other range
      return;
    }
    
    /* Finite state machine for hardware autoranging */
    switch (range)
    {
      case CurrentRange_HighCurrent:        
        if (adcErrorCounter != ADCError->errorCounter) /* Positive current overload is taken from ADC error message */
        {
          adcErrorCounter = ADCError->errorCounter;
          if (ADCError->error == ErrorMessaging_ADC_Overload)
          {
              AmmeterError.errorCounter++;
              AmmeterError.error = ErrorMessaging_Ammeter_CurrentOverload;
          }          
        }        
            
        /* calculate new voltage value and save it to local variable "signed current" */
        signedCurrent = (((int64_t)(AMMETER_SLOPE_HI)) * ((int64_t)(ADCRaw->value))) / (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB) + AMMETER_OFFSET_HI;   
        signedUnfilteredCurrent = (((int64_t)(AMMETER_SLOPE_HI)) * ((int64_t)(ADCRaw->unfilteredValue))) / (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB) + AMMETER_OFFSET_HI;
      break;
      case CurrentRange_LowCurrent:
        if (adcErrorCounter != ADCError->errorCounter) /* ADC overload in low current range only switches to high current range, without updating the current value */
        {
          adcErrorCounter = ADCError->errorCounter;
          if ((ADCError->error == ErrorMessaging_ADC_Overload) && (Control_GetCCCV() != Control_CCCV_CC))
          {
            RangeSwitcher_SetCurrentRange(CurrentRange_HighCurrent);            
            resetFilter = RangeSwitcher_HasCurrentRangeChanged(); // Reset filter to remove values measured at other range
            return;
          }          
        }
      
        /* calculate new voltage value and save it to local variable "signed current" */
        signedCurrent = (((int64_t)(AMMETER_SLOPE_LO)) * ((int64_t)(ADCRaw->value))) / (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB) + AMMETER_OFFSET_LO;    
        signedUnfilteredCurrent = (((int64_t)(AMMETER_SLOPE_LO)) * ((int64_t)(ADCRaw->unfilteredValue))) / (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB) + AMMETER_OFFSET_LO;     
      break;
      default:
      return;
    }
    
    if (signedUnfilteredCurrent < AMMETER_MINIMUM_CURRENT)
    {
      /* Signal negative current */
      AmmeterError.errorCounter++;      
      AmmeterError.error = ErrorMessaging_Ammeter_NegativeCurrent;
    }

    uint32_t newFilteredCurrent;
    uint32_t newUnfilteredCurrent;
    
    if (signedCurrent < 0)
    {
      newFilteredCurrent = 0; /* Negative current not reported */
    }
    else
    {
      newFilteredCurrent = (uint32_t)signedCurrent;
    }

    if (signedUnfilteredCurrent < 0)
    {
      newUnfilteredCurrent = 0; /* Negative current not reported */
    }
    else
    {
      newUnfilteredCurrent = (uint32_t)signedUnfilteredCurrent;
    }

    if (Control_GetCCCV() != Control_CCCV_CC)
    {  
      if (newUnfilteredCurrent > AMMETER_HYSTERESIS_UP)
      {
        range = CurrentRange_HighCurrent;
      }
      else if (newUnfilteredCurrent < AMMETER_HYSTERESIS_DOWN)
      {
        range = CurrentRange_LowCurrent;
      }
      RangeSwitcher_SetCurrentRange(range);
    }

    resetFilter |= RangeSwitcher_HasCurrentRangeChanged(); // Reset filter to remove values measured at other range

    if (false == resetFilter)
    {
      current.value = newFilteredCurrent;
      current.unfilteredValue = newUnfilteredCurrent;
      current.counter++;
      current.milliseconds = millis();
    }
  }
}

void Ammeter_SetSpeed(Measurement_Speeds msp)
{
  ADC_SetupChannel(ADC_I, Measurement_Speed[msp]);  
}

const TSCADCULong * Ammeter_GetCurrent(void)
{
  return &current;  
}

const ErrorMessaging_Error * Ammeter_GetError(void)
{
  return &AmmeterError;
}

/* </Implementations> */ 
