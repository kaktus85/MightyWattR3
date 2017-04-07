/**
 * ErrorMessaging.cpp
 *
 * 2016-11-04
 * kaktus circuits
 * GNU GPL v.3
 */
 
/* <Includes> */ 

#include "AD569xR.h"
#include "ADC.h"
#include "ADS1x15.h"
#include "Ammeter.h"
#include "Communication.h"
#include "Control.h"
#include "CurrentSetter.h"
#include "DACC.h"
#include "Limiter.h"
#include "Measurement.h"
#include "Thermometer.h"
#include "VoltageSetter.h"
#include "Voltmeter.h"

#include "ErrorMessaging.h"

#include "Arduino.h"
#include "MightyWatt.h"

/* </Includes> */ 


/* <Module variables> */ 

static const char ADC_Overload[] FLASHMEMORY = "ADC overload";
static const char ADC_NotResponding[] FLASHMEMORY = "ADC not responding";
#ifdef ADC_TYPE_ADS1115
  static const char ADS1x15_ResultNotReady[] FLASHMEMORY = "ADS1115 requested result not ready";
#elif defined(ADC_TYPE_ADS1015)
  static const char ADS1x15_ResultNotReady[] FLASHMEMORY = "ADS1015 requested result not ready";
#else
  #error No ADC defined
#endif
static const char AD569xR_Overload[] FLASHMEMORY = "AD569xR overload";
static const char Ammeter_CurrentOverload[] FLASHMEMORY = "Ammeter overload";
static const char Ammeter_NegativeCurrent[] FLASHMEMORY = "Ammeter negative current detected";
static const char Communication_CommandTimeout[] FLASHMEMORY = "Command timeout";
static const char CurrentSetter_SetCurrentOverload[] FLASHMEMORY = "Requested current cannot be set";
static const char DACC_Overload[] FLASHMEMORY = "DAC overload";
static const char DACC_UpperLimitReached[] FLASHMEMORY = "DAC upper limit reached";
static const char DACC_LowerLimitReached[] FLASHMEMORY = "DAC lower limit reached";
static const char Limiter_CurrentOverload[] FLASHMEMORY = "Current overload";
static const char Limiter_VoltageOverload[] FLASHMEMORY = "Voltage overload";
static const char Limiter_PowerOverload[] FLASHMEMORY = "Power overload";
static const char Limiter_SOAExceeded[] FLASHMEMORY = "Safe operating area exceeded";
static const char Limiter_Overheat[] FLASHMEMORY = "Overheat";
static const char Limiter_HardwareFault[] FLASHMEMORY = "Hardware fault";
static const char Measurement_Invalid[] FLASHMEMORY = "Measurement invalid";
static const char Thermometer_HardwareFault[] FLASHMEMORY = "Thermometer fault";
static const char VoltageSetter_SetVoltageOverload[] FLASHMEMORY = "Requested voltage cannot be set";
static const char Voltmeter_VoltageOverload[] FLASHMEMORY = "Voltmeter overload";
static const char Voltmeter_NegativeVoltage[] FLASHMEMORY = "Voltmeter negative voltage detected";

const char * ErrorMessaging_ErrorNames[] = 
{
  ADC_Overload, ADC_NotResponding, ADS1x15_ResultNotReady, AD569xR_Overload, Ammeter_CurrentOverload, Ammeter_NegativeCurrent,
  Communication_CommandTimeout, CurrentSetter_SetCurrentOverload, DACC_Overload, DACC_UpperLimitReached, DACC_LowerLimitReached,
  Limiter_CurrentOverload, Limiter_VoltageOverload, Limiter_PowerOverload, Limiter_SOAExceeded, Limiter_Overheat, Limiter_HardwareFault,
  Measurement_Invalid, Thermometer_HardwareFault, VoltageSetter_SetVoltageOverload, Voltmeter_VoltageOverload, Voltmeter_NegativeVoltage
};

static const ErrorMessaging_Error * AD569xRError;
static uint8_t AD569xRErrorCounter;

static const ErrorMessaging_Error * ADCError[ADC_CHANNEL_COUNT];
static uint8_t ADCErrorCounter[ADC_CHANNEL_COUNT];

static const ErrorMessaging_Error * ADS1x15Error;
static uint8_t ADS1x15ErrorCounter;

static const ErrorMessaging_Error * AmmeterError;
static uint8_t AmmeterErrorCounter;

static const ErrorMessaging_Error * CommunicationError;
static uint8_t CommunicationErrorCounter;

static const ErrorMessaging_Error * ControlError;
static uint8_t ControlErrorCounter;

static const ErrorMessaging_Error * CurrentSetterError;
static uint8_t CurrentSetterErrorCounter;

static const ErrorMessaging_Error * DACError;
static uint8_t DACErrorCounter;

static const ErrorMessaging_Error * LimiterError;
static uint8_t LimiterErrorCounter;

static const ErrorMessaging_Error * MeasurementError;
static uint8_t MeasurementErrorCounter;

static const ErrorMessaging_Error * ThermometerError;
static uint8_t ThermometerErrorCounter;

static const ErrorMessaging_Error * VoltageSetterError;
static uint8_t VoltageSetterErrorCounter;

static const ErrorMessaging_Error * VoltmeterError;
static uint8_t VoltmeterErrorCounter;

/* </Module variables> */ 


/* <Implementations> */ 

void ErrorMessaging_Init(void)
{  
  uint8_t i;
  
  AD569xRError = AD569xR_GetError();
  AD569xRErrorCounter = AD569xRError->errorCounter;
  
  for (i = 0; i < ADC_CHANNEL_COUNT; i++)
  {
    ADCError[i] = ADC_GetError((ADC_Channels)i);
    ADCErrorCounter[i] = ADCError[i]->errorCounter;
  }

  ADS1x15Error = ADS1x15_GetError();
  ADS1x15ErrorCounter = ADS1x15Error->errorCounter;
  
  AmmeterError = Ammeter_GetError();
  AmmeterErrorCounter = AmmeterError->errorCounter;
  
  CommunicationError = Communication_GetError();
  CommunicationErrorCounter = CommunicationError->errorCounter;
  
  ControlError = Control_GetError();
  ControlErrorCounter = ControlError->errorCounter;
  
  CurrentSetterError = CurrentSetter_GetError();
  CurrentSetterErrorCounter = CurrentSetterError->errorCounter;
  
  DACError = DACC_GetError();
  DACErrorCounter = DACError->errorCounter;
  
  LimiterError = Limiter_GetError();
  LimiterErrorCounter = LimiterError->errorCounter;
  
  MeasurementError = Measurement_GetError();
  MeasurementErrorCounter = MeasurementError->errorCounter;
  
  ThermometerError = Thermometer_GetError();
  ThermometerErrorCounter = ThermometerError->errorCounter;
  
  VoltageSetterError = VoltageSetter_GetError();
  VoltageSetterErrorCounter = VoltageSetterError->errorCounter;
  
  VoltmeterError = Voltmeter_GetError();
  VoltmeterErrorCounter = VoltmeterError->errorCounter;
}

uint32_t ErrorMessaging_GetErrorFlags(void)
{
  uint8_t i;
  uint32_t flag = 0;  
  
  if (AD569xRErrorCounter != AD569xRError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(AD569xRError->error));
    AD569xRErrorCounter = AD569xRError->errorCounter;
  }
  
  for (i = 0; i < ADC_CHANNEL_COUNT; i++)
  {
    if (ADCErrorCounter[i] != ADCError[i]->errorCounter)
    {
      flag |= 1UL << ((uint8_t)(ADCError[i]->error));
      ADCErrorCounter[i] = ADCError[i]->errorCounter;
    }
  }
  
  if (ADS1x15ErrorCounter != ADS1x15Error->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(ADS1x15Error->error));
    ADS1x15ErrorCounter = ADS1x15Error->errorCounter;
  }
  
  if (AmmeterErrorCounter != AmmeterError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(AmmeterError->error));
    AmmeterErrorCounter = AmmeterError->errorCounter;
  }
  
  if (CommunicationErrorCounter != CommunicationError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(CommunicationError->error));
    CommunicationErrorCounter = CommunicationError->errorCounter;
  }
  
  if (ControlErrorCounter != ControlError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(ControlError->error));
    ControlErrorCounter = ControlError->errorCounter;
  }
  
  if (CurrentSetterErrorCounter != CurrentSetterError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(CurrentSetterError->error));
    CurrentSetterErrorCounter = CurrentSetterError->errorCounter;
  }
  
  if (DACErrorCounter != DACError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(DACError->error));
    DACErrorCounter = DACError->errorCounter;
  }
  
  if (LimiterErrorCounter != LimiterError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(LimiterError->error));
    LimiterErrorCounter = LimiterError->errorCounter;
  }
  
  if (MeasurementErrorCounter != MeasurementError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(MeasurementError->error));
    MeasurementErrorCounter = MeasurementError->errorCounter;
  }
  
  if (ThermometerErrorCounter != ThermometerError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(ThermometerError->error));
    ThermometerErrorCounter = ThermometerError->errorCounter;
  }
    
  if (VoltageSetterErrorCounter != VoltageSetterError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(VoltageSetterError->error));
    VoltageSetterErrorCounter = VoltageSetterError->errorCounter;
  }
  
  if (VoltmeterErrorCounter != VoltmeterError->errorCounter)
  {
    flag |= 1UL << ((uint8_t)(VoltmeterError->error));
    VoltmeterErrorCounter = VoltmeterError->errorCounter;
  }
  
  return flag;
}

uint8_t ErrorMessaging_ErrorNamesCount(void)
{
  return sizeof(ErrorMessaging_ErrorNames)/sizeof(char *);
}

void ErrorMessaging_GetError(uint8_t errorNumber, char * message)
{
  uint16_t i = 0;  
  
  do
  {
    #ifdef UNO
      message[i] = pgm_read_byte(&(ErrorMessaging_ErrorNames[errorNumber])[i]);
    #elif defined(ZERO)
      message[i] = (ErrorMessaging_ErrorNames[errorNumber])[i];
    #else
      #error No platform defined
    #endif
    i++;
  }
  while(message[i - 1] != 0);
}

/* </Implementations> */
