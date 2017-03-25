/**
 * FanController.cpp
 *
 * 2016-11-01
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "Fan.h"
#include "Communication.h"
#include "Measurement.h"
#include "Thermometer.h"
#include "FanController.h"

/* </Includes> */ 


/* <Module variables> */ 

static const Communication_WriteCommand * writeCommand; /* Pointer to the write command where new data from communication can be found */
static const Measurement_Values * measurementValues; /* Pointer to the latest measured voltage, current, power and resistance */
static const TSCUChar * temperature; /* Pointer to structure where temperature can be found */
static uint8_t commandCounter/*, measurementCounter, temperatureCounter*/; /* Number of the last executed command from communication, number of the last measurement data, number of the last temperature data */
static FanController_Rules FanRules; /* Describes under which circumstances the fan will be on and off */
void (* FanController_Keep)(void); /* Pointer to the constant keeper function */
static uint32_t FanStartTime; /* Time when fan started, to avoid excessive on/off switching */

/* </Module variables> */ 


/* <Declarations (prototypes)> */ 

/**
 * Enforces fan rules and manages fan on/off
 */
void FanController_KeepRule(void);

/* </Declarations (prototypes)> */ 


/* <Implementations> */ 

void FanController_Init(void)
{
  writeCommand = Communication_GetWriteCommand();  
  measurementValues = Measurement_GetValues();
  temperature = Thermometer_GetTemperature();
  commandCounter = 0;
  FanRules = FAN_CONTROLLER_DEFAULT_RULE;
  FanController_Keep = &FanController_KeepRule;
  FanStartTime = 0;
}

void FanController_Do(void)
{
  /* Check new command */
  if (writeCommand->commandCounter != commandCounter)
  {
    /* LSB first */
    switch (writeCommand->command)
    {
      case WriteCommand_FanRules:
      {
        uint8_t newRules = (writeCommand->data)[0];
        if (newRules < FAN_CONTROLLER_RULES_COUNT)
        {
          FanRules = (FanController_Rules)newRules;
          FanController_Keep = &FanController_KeepRule;
          FanStartTime = millis() - FAN_CONTROLLER_MINIMUM_ONTIME; /* allows immediate change upon receiving command */
        }          
        break;
      }
      default:
      /* command handled by other modules */
      break;
    }
    commandCounter = writeCommand->commandCounter;
  }  
  
  if (FanController_Keep != NULL)
  {
    FanController_Keep();
  }
}

void FanController_KeepRule(void)
{ 
  static uint8_t measurementCounter = 0, temperatureCounter = 0; /* Number of the last executed command from communication, number of the last measurement data, number of the last temperature data */  
  
  switch (FanRules)
  {
    case FanRule_AutoHigh:
      if ((measurementCounter != measurementValues->counter) && (temperatureCounter != temperature->counter)) /* Only process new values */
      {
        if ((temperature->value > FAN_CONTROLLER_AUTOHIGH_TEMP_UP) || (measurementValues->power > FAN_CONTROLLER_AUTOHIGH_P_UP))
        {
          Fan_Set(Fan_On);
          FanStartTime = millis();
        }
        else if ((temperature->value < FAN_CONTROLLER_AUTOHIGH_TEMP_DOWN) && (measurementValues->power < FAN_CONTROLLER_AUTOHIGH_P_DOWN) && ((millis() - FanStartTime) > FAN_CONTROLLER_MINIMUM_ONTIME))
        {
          Fan_Set(Fan_Off);  
        }
        /* Otherwise no change */
        
        measurementCounter = measurementValues->counter;
        temperatureCounter = temperature->counter;
      }        
    break;
    case FanRule_AutoLow:
      if ((measurementCounter != measurementValues->counter) && (temperatureCounter != temperature->counter)) /* Only process new values */
      {
        if ((temperature->value > FAN_CONTROLLER_AUTOLOW_TEMP_UP) || (measurementValues->power > FAN_CONTROLLER_AUTOLOW_P_UP))
        {
          Fan_Set(Fan_On);
          FanStartTime = millis();
        }
        else if ((temperature->value < FAN_CONTROLLER_AUTOLOW_TEMP_DOWN) && (measurementValues->power < FAN_CONTROLLER_AUTOLOW_P_DOWN) && ((millis() - FanStartTime) > FAN_CONTROLLER_MINIMUM_ONTIME))
        {
          Fan_Set(Fan_Off);  
        }
        /* Otherwise no change */
        
        measurementCounter = measurementValues->counter;
        temperatureCounter = temperature->counter;
      }
    break;
    case FanRule_AlwaysOn:
    default:
      Fan_Set(Fan_On);
      FanStartTime = millis();
    break;
  }
}

/* </Implementations> */ 
