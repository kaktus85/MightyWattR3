/**
 * LEDController.cpp
 *
 * 2016-10-31
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "LED.h"
#include "Communication.h"
#include "Measurement.h"
#include "Thermometer.h"
#include "LEDController.h"
#include "Configuration.h"
#include "Voltmeter.h"
#include "Ammeter.h"

/* </Includes> */ 


/* <Module variables> */ 

static const Communication_WriteCommand * writeCommand; /* Pointer to the write command where new data from communication can be found */
static const Measurement_Values * measurementValues; /* Pointer to the latest measured voltage, current, power and resistance */
static const TSCUChar * temperature; /* Pointer to structure where temperature can be found */
static uint8_t commandCounter, measurementCounter, temperatureCounter; /* Number of the last executed command from communication, number of the last measurement data, number of the last temperature data */
static uint8_t LEDBrightness; /* Indicates the brightness of the LED when on*/
static uint8_t LEDLightRules; /* Describes under which circumstances the LED will light */
void (* LEDController_Keep)(void); /* Pointer to the constant keeper function */

/* </Module variables> */ 


/* <Declarations (prototypes)> */ 

/**
 * Enforces LED rules and manages LED lighting
 */
void LEDController_KeepRule(void);

/* </Declarations (prototypes)> */ 


/* <Implementations> */ 

void LEDController_Init(void)
{
  writeCommand = Communication_GetWriteCommand();  
  measurementValues = Measurement_GetValues();
  temperature = Thermometer_GetTemperature();
  commandCounter = 0;
  measurementCounter = 0;
  temperatureCounter = 0;
  LEDBrightness = LED_CONTROLLER_DEFAULT_BRIGHTNESS;
  LEDLightRules = LED_CONTROLLER_DEFAULT_RULE;
  LEDController_Keep = &LEDController_KeepRule;
}

void LEDController_Do(void)
{
  /* Check new command */
  if (writeCommand->commandCounter != commandCounter)
  {
    /* LSB first */
    switch (writeCommand->command)
    {
      case WriteCommand_LEDRules:
        LEDLightRules = (writeCommand->data)[0];        
        LEDController_Keep = &LEDController_KeepRule;
      break;
      case WriteCommand_LEDBrightness:
        LEDBrightness = (writeCommand->data)[0];
      break;
      default:
      /* command handled by other modules */
      break;
    }
    commandCounter = writeCommand->commandCounter;
  }  
  
  if (LEDController_Keep != NULL)
  {
    LEDController_Keep();
  }
}

void LEDController_KeepRule(void)
{
  static uint8_t LEDWord = 0; /* LED rule flag bit word, bits correspond to rules being activated (LEDWord > 0 will trigger LED on) */
  
  if (LEDLightRules == LEDRule_AlwaysOff) /* No rules - LED always off */
  {
    LED_Off(); 
    return;
  }
  
  if ((LEDLightRules & LEDRule_AlwaysOn) > 0) /* LED always on */
  {
    LEDWord |= LEDRule_AlwaysOn;
  }  
  else
  {
    LEDWord &= ~LEDRule_AlwaysOn;
  }
   
  if (measurementCounter != measurementValues->counter) /* Only process new values */
  {
    /* Power > 1 % rule */ 
    if ((LEDLightRules & LEDRule_P1) > 0)
    {
      if (measurementValues->power > (MAXIMUM_POWER / LED_CONTROLLER_P1_UP))
      {
        LEDWord |= LEDRule_P1; /* Set flag (power over threshold) */
      }
      else if (measurementValues->power < (MAXIMUM_POWER / LED_CONTROLLER_P1_DOWN))
      {
        LEDWord &= ~LEDRule_P1; /* Clear flag (power below hysteresis) */
      }           
    }
    else
    {
      LEDWord &= ~LEDRule_P1;
    }
    
    /* Voltage > 1 % rule */ 
    if ((LEDLightRules & LEDRule_V1) > 0)
    {
      if (measurementValues->voltage > (VOLTMETER_MAXIMUM_VOLTAGE / LED_CONTROLLER_V1_UP))
      {
        LEDWord |= LEDRule_V1; /* Set flag (power over threshold) */
      }
      else if (measurementValues->voltage < (VOLTMETER_MAXIMUM_VOLTAGE / LED_CONTROLLER_V1_DOWN))
      {
        LEDWord &= ~LEDRule_V1; /* Clear flag (power below hysteresis) */
      }           
    }
    else
    {
      LEDWord &= ~LEDRule_V1;
    }
    
    /* Current > 1 % rule */ 
    if ((LEDLightRules & LEDRule_I1) > 0)
    {
      if (measurementValues->current > (AMMETER_MAXIMUM_CURRENT / LED_CONTROLLER_I1_UP))
      {
        LEDWord |= LEDRule_I1; /* Set flag (power over threshold) */
      }
      else if (measurementValues->current < (AMMETER_MAXIMUM_CURRENT / LED_CONTROLLER_I1_DOWN))
      {
        LEDWord &= ~LEDRule_I1; /* Clear flag (power below hysteresis) */
      }           
    }
    else
    {
      LEDWord &= ~LEDRule_I1;
    }
    
    /* Power > 10 % rule */ 
    if ((LEDLightRules & LEDRule_P10) > 0)
    {
      if (measurementValues->power > (MAXIMUM_POWER / LED_CONTROLLER_P10_UP))
      {
        LEDWord |= LEDRule_P10; /* Set flag (power over threshold) */
      }
      else if (measurementValues->power < (MAXIMUM_POWER / LED_CONTROLLER_P10_DOWN))
      {
        LEDWord &= ~LEDRule_P10; /* Clear flag (power below hysteresis) */
      }           
    }
    else
    {
      LEDWord &= ~LEDRule_P10;
    }
    
    /* Voltage > 10 % rule */ 
    if ((LEDLightRules & LEDRule_V10) > 0)
    {
      if (measurementValues->voltage > (VOLTMETER_MAXIMUM_VOLTAGE / LED_CONTROLLER_V10_UP))
      {
        LEDWord |= LEDRule_V10; /* Set flag (power over threshold) */
      }
      else if (measurementValues->voltage < (VOLTMETER_MAXIMUM_VOLTAGE / LED_CONTROLLER_V10_DOWN))
      {
        LEDWord &= ~LEDRule_V10; /* Clear flag (power below hysteresis) */
      }           
    }
    else
    {
      LEDWord &= ~LEDRule_V10;
    }
    
    /* Current > 10 % rule */ 
    if ((LEDLightRules & LEDRule_I10) > 0)
    {
      if (measurementValues->current > (AMMETER_MAXIMUM_CURRENT / LED_CONTROLLER_I10_UP))
      {
        LEDWord |= LEDRule_I10; /* Set flag (power over threshold) */
      }
      else if (measurementValues->current < (AMMETER_MAXIMUM_CURRENT / LED_CONTROLLER_I10_DOWN))
      {
        LEDWord &= ~LEDRule_I10; /* Clear flag (power below hysteresis) */
      }           
    }
    else
    {
      LEDWord &= ~LEDRule_I10;
    }
    
    measurementCounter = measurementValues->counter;
  }
  
  if (temperatureCounter != temperature->counter) /* Only process new values */
  {
    /* Temperature rule */       
    if ((LEDLightRules & LEDRule_T50) > 0)
    {
      if (temperature->value > LED_CONTROLLER_T50_UP)
      {
        LEDWord |= LEDRule_T50; /* Set flag (temperature over threshold) */
      }
      else if (temperature->value < LED_CONTROLLER_T50_DOWN)
      {
        LEDWord &= ~LEDRule_T50; /* Clear flag (temperature below hysteresis) */
      }            
    }
    else
    {
      LEDWord &= ~LEDRule_T50;
    }
    temperatureCounter = temperature->counter;
  }
  
  /* Apply rules */
  if (LEDWord > 0)
  {
    LED_Set(LEDBrightness);
  }
  else
  {
    LED_Off();
  }
}

/* </Implementations> */ 
