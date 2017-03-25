/**
 * Voltmeter.cpp
 *
 * 2016-10-21
 * kaktus circuits
 * GNU GPL v.3
 */
 

/* <Includes> */ 

#include "Arduino.h"
#include "Voltmeter.h"
#include "ADC.h"
#include "Configuration.h"
#include "Communication.h"
#include "DACC.h"
#include "RangeSwitcher.h"
#include "Control.h"

/* </Includes> */ 


/* <Declarations (prototypes)> */ 

/**
 * Processes ADC values
 * Part of the "Do" loop
 */ 
void Voltmeter_ProcessADC(void);

/**
 * Processes communication command
 * Part of the "Do" loop
 */ 
void Voltmeter_ProcessCommunication(void);

/**
 * Sets 2-terminal or 4-terminal voltage measurement mode
 *
 * @param mode - New mode to set
 */
void Voltmeter_SetMode(Voltmeter_Modes mode);

/* </Declarations (prototypes)> */ 


/* <Module variables> */ 

static TSCADCULong voltage; /* contains voltage in microvolts */
static Voltmeter_Modes voltmeter_mode; /* 2-terminal or 4-terminal */
static const TSCADCLong * ADCRaw;
static uint8_t adcCounter, adcErrorCounter, commandCounter;
static ErrorMessaging_Error VoltmeterError;
const static ErrorMessaging_Error * ADCError;
const static Communication_WriteCommand * writeCommand;

/* </Module variables> */ 


/* <Implementations> */ 

void Voltmeter_Init(void)
{ 
  pinMode(VOLTMETER_4TERMINAL_PIN, OUTPUT);
  Voltmeter_SetSpeed(VOLTMETER_DEFAULT_MEASUREMENT_SPEED);
  Voltmeter_SetMode(VOLTMETER_DEFAULT_MODE);
  ADCRaw = ADC_GetVoltage(ADC_V);
  adcCounter = ADCRaw->counter;
  voltage.counter = 0;
  voltage.milliseconds = 0;
  voltage.value = 0;
  VoltmeterError.errorCounter = 0;
  VoltmeterError.error = ErrorMessaging_Voltmeter_VoltageOverload;
  ADCError = ADC_GetError(ADC_V);
  adcErrorCounter = ADCError->errorCounter;
  writeCommand = Communication_GetWriteCommand();
  commandCounter = writeCommand->commandCounter;
}

void Voltmeter_Do(void)
{   
  Voltmeter_ProcessADC();
  Voltmeter_ProcessCommunication();
}

void Voltmeter_ProcessADC(void)
{
  RangeSwitcher_VoltageRanges range = RangeSwitcher_GetVoltageRange();
  
  if (adcCounter != ADCRaw->counter) /* Process only new reading from ADC */
  {  
    adcCounter = ADCRaw->counter;
    int32_t signedVoltage, signedUnfilteredVoltage;

    /* Finite state machine for hardware autoranging */
    switch (range)
    {
      case VoltageRange_HighVoltage:               
        if (adcErrorCounter != ADCError->errorCounter) /* Positive voltage overload is taken from ADC error message */
        {
          adcErrorCounter = ADCError->errorCounter;
          if (ADCError->error == ErrorMessaging_ADC_Overload)
          {
            VoltmeterError.errorCounter++;
            VoltmeterError.error = ErrorMessaging_Voltmeter_VoltageOverload;
          }
        }

        /* calculate new voltage value and save it to local variable "voltage" */ 
        signedVoltage = (((int64_t)(VOLTMETER_SLOPE_HI)) * ((int64_t)(ADCRaw->value))) / (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB) + VOLTMETER_OFFSET_HI;
        signedUnfilteredVoltage = (((int64_t)(VOLTMETER_SLOPE_HI)) * ((int64_t)(ADCRaw->unfilteredValue))) / (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB) + VOLTMETER_OFFSET_HI;
      break;
      case VoltageRange_LowVoltage:
        /* ADC overload in low voltage range will only switch to high voltage range*/
        if (adcErrorCounter != ADCError->errorCounter)
        {
          adcErrorCounter = ADCError->errorCounter; 
          if ((ADCError->error == ErrorMessaging_ADC_Overload) && (Control_GetCCCV() != Control_CCCV_CV))
          {   
            RangeSwitcher_SetVoltageRange(VoltageRange_HighVoltage);
            return;
          }      
        }
        /* calculate new voltage value and save it to local variable "voltage" */
        signedVoltage = (((int64_t)(VOLTMETER_SLOPE_LO)) * ((int64_t)(ADCRaw->value))) / (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB) + VOLTMETER_OFFSET_LO;
        signedUnfilteredVoltage = (((int64_t)(VOLTMETER_SLOPE_LO)) * ((int64_t)(ADCRaw->unfilteredValue))) / (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB) + VOLTMETER_OFFSET_LO;
      break;
      default:
      return;
    }
    
    if (signedUnfilteredVoltage < VOLTMETER_MINIMUM_VOLTAGE)
    {
      /* Signal negative voltage */
      VoltmeterError.errorCounter++;
      VoltmeterError.error = ErrorMessaging_Voltmeter_NegativeVoltage;
    }
    
    if (signedVoltage < 0)
    {
      voltage.value = 0; /* Negative voltage not reported */
    }
    else
    {
      voltage.value = (uint32_t)signedVoltage;
    }

    if (signedUnfilteredVoltage < 0)
    {
      voltage.unfilteredValue = 0; /* Negative voltage not reported */
    }
    else
    {
      voltage.unfilteredValue = (uint32_t)signedUnfilteredVoltage;
    }

    /* Calculate range if not in CV mode */
    if (Control_GetCCCV() != Control_CCCV_CV)
    {  
      if (voltage.unfilteredValue > VOLTMETER_HYSTERESIS_UP)
      {
        range = VoltageRange_HighVoltage;
      }
      else if (voltage.unfilteredValue < VOLTMETER_HYSTERESIS_DOWN)
      {
        range = VoltageRange_LowVoltage;
      }
      RangeSwitcher_SetVoltageRange(range);
    }
    
    voltage.counter++;
    voltage.milliseconds = millis();
  }
}

void Voltmeter_ProcessCommunication(void)
{
  /* Process new communication command - set mode*/
  if (commandCounter != writeCommand->commandCounter)
  {
    /* LSB first */
    switch (writeCommand->command)
    {
      case WriteCommand_4Wire:
        if ((writeCommand->data)[0] == 0)
        {
          Voltmeter_SetMode(Voltmeter_2Terminal);
        }
        else if ((writeCommand->data)[0] == 1)
        {
          Voltmeter_SetMode(Voltmeter_4Terminal);
        }
      break;
      default:
      break;
    }
    
    commandCounter = writeCommand->commandCounter;
  }
}

void Voltmeter_SetSpeed(Measurement_Speeds msp)
{  
  ADC_SetupChannel(ADC_V, Measurement_Speed[msp]);
}

void Voltmeter_SetMode(Voltmeter_Modes mode)
{  
  voltmeter_mode = mode;
  
  switch (voltmeter_mode)
  {
    case Voltmeter_2Terminal:
      digitalWrite(VOLTMETER_4TERMINAL_PIN, LOW);
    break;
    case Voltmeter_4Terminal:
      digitalWrite(VOLTMETER_4TERMINAL_PIN, HIGH);
    break;
    default:
    break;
  }  
}

const TSCADCULong * Voltmeter_GetVoltage(void)
{
  return &voltage;  
}

Voltmeter_Modes Voltmeter_GetMode(void)
{
  return voltmeter_mode;
}

const ErrorMessaging_Error * Voltmeter_GetError(void)
{
  return &VoltmeterError;
}

/* </Implementations> */ 
