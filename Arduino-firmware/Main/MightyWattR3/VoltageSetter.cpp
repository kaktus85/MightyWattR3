/**
 * VoltageSetter.cpp
 *
 * 2016-10-29
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "Control.h"
#include "DACC.h"
#include "Voltmeter.h"
#include "VoltageSetter.h"
#include "RangeSwitcher.h"

/* </Includes> */ 


/* <Module variables> */ 

//static uint16_t dacTargetValue;
static ErrorMessaging_Error VoltageSetterError;
static uint32_t presentVoltage;

/* </Module variables> */ 


/* <Implementations> */ 

void VoltageSetter_Init(void)
{
  VoltageSetterError.errorCounter = 0;
  VoltageSetterError.error = ErrorMessaging_VoltageSetter_SetVoltageOverload;
}

void VoltageSetter_Do(void)
{
  uint32_t dac;
  
//  if (presentVoltage > 0) /* on zero voltage set true zero to DAC */
//  {     
    switch (RangeSwitcher_GetVoltageRange())
    {
      case VoltageRange_HighVoltage:
        dac = (((uint64_t)((int32_t)presentVoltage + VOLTSETTER_OFFSET_HI)) << 16) / VOLTSETTER_SLOPE_HI;
        if (dac > DAC_MAXIMUM) /* Set voltage higher than maximum */
        {
          VoltageSetterError.errorCounter++;
          VoltageSetterError.error = ErrorMessaging_VoltageSetter_SetVoltageOverload;
          dac = DAC_MAXIMUM;
        }
      break;
      case VoltageRange_LowVoltage:
        dac = (((uint64_t)((int32_t)presentVoltage + VOLTSETTER_OFFSET_LO)) << 16) / VOLTSETTER_SLOPE_LO;
        if (dac > DAC_MAXIMUM)
        {
          RangeSwitcher_VoltageRangeUp(); // increase range and call this function recursively
          VoltageSetter_Do();
          return;
        }
      break;
      default:      
      return;
    }
//  }
//  else
//  {
//    dac = 0;
//  }
  
  if (!DACC_SetVoltage(dac & 0xFFFF)) /* Set calculated voltage */
  {
    VoltageSetterError.errorCounter++;
    VoltageSetterError.error = ErrorMessaging_VoltageSetter_SetVoltageOverload; 
  }  
  Control_SetCCCV(Control_CCCV_CV); /* Set phase CV */
}

//void VoltageSetter_Do(void)
//{
//  uint16_t newValue;
//  uint16_t presentValue = DACC_GetValue();
//  
//  if ((dacTargetValue != presentValue) && (Control_GetCCCV() == Control_CCCV_CV))
//  {
//    if (dacTargetValue < presentValue)
//    {
//      if ((presentValue - dacTargetValue) > VOLTAGE_SETTER_MAXIMUM_STEP)
//      {
//        newValue = presentValue - VOLTAGE_SETTER_MAXIMUM_STEP;
//      }
//      else
//      {
//        newValue = dacTargetValue;
//      }
//    }
//    else
//    {
//      if ((dacTargetValue - presentValue) > VOLTAGE_SETTER_MAXIMUM_STEP)
//      {
//        newValue = presentValue + VOLTAGE_SETTER_MAXIMUM_STEP;
//      }
//      else
//      {
//        newValue = dacTargetValue;
//      }
//    }
//    
//    DACC_SetVoltage(newValue);
//  }
//}

void VoltageSetter_SetVoltage(uint32_t voltage)
{
  presentVoltage = voltage;
}

uint32_t VoltageSetter_GetVoltage(void)
{
  return presentVoltage;
}

void VoltageSetter_Plus(uint32_t value)
{
  if ((presentVoltage + value) < presentVoltage || (presentVoltage + value) < value) // overflow check
  {  
    VoltageSetter_SetVoltage(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE);
  }
  else
  {
    VoltageSetter_SetVoltage(value + presentVoltage);
  }
}

void VoltageSetter_Minus(uint32_t value)
{
  if (value > presentVoltage) // overflow check
  {  
    VoltageSetter_SetVoltage(0);
  }
  else
  {
    VoltageSetter_SetVoltage(presentVoltage - value); 
  }
}
//
//bool VoltageSetter_SetVoltage(uint32_t voltage)
//{  
//  uint32_t dac;
//  
//  if (voltage > 0) /* on zero voltage set true zero to DAC */
//  {      
//    switch (Voltmeter_GetRange())
//    {
//      case Voltmeter_HighVoltage:
//        if ((int32_t)voltage > 0)
//        {
//          dac = (((uint64_t)((int32_t)voltage + VOLTSETTER_OFFSET_HI)) << 16) / VOLTSETTER_SLOPE_HI;
//        }
//        else
//        {
//          dac = 0;
//        }
//      break;
//      case Voltmeter_LowVoltage:
//        if ((int32_t)voltage > 0)
//        {
//          dac = (((uint64_t)((int32_t)voltage + VOLTSETTER_OFFSET_LO)) << 16) / VOLTSETTER_SLOPE_LO;
//          if (dac > DAC_MAXIMUM) /* Set maximum DAC at low range and wait for automatic range switch */
//          {
//            dac = DAC_MAXIMUM;
//          }
//        }
//        else
//        {
//          dac = 0;
//        }
//      break;
//      default:
//      return false;
//    }
//    
//  }
//  else
//  {
//    dac = 0;
//  }
//
//  //dacTargetValue = dac & 0xFFFF;
//  
////  if (Control_GetCCCV() != Control_CCCV_CV)
////  {
////    Control_SetCCCV(Control_CCCV_CV); /* Set phase CV */
////  }
//  
//  Control_SetCCCV(Control_CCCV_CV); /* Set phase CV */
//  if (!DACC_SetVoltage(dac & 0xFFFF))
//  {
//    VoltageSetterError.errorCounter++;
//    VoltageSetterError.error = ErrorMessaging_VoltageSetter_SetVoltageOverload;
//    return false;
//  }   
//  else
//  {
//    return true;
//  }
//}
//
//void VoltageSetter_SetMaximumVoltage(void)
//{
//  VoltageSetter_SetPercentOfRange(100);
//}
//
//void VoltageSetter_SetPercentOfRange(uint8_t percentage)
//{  
//  Control_SetCCCV(Control_CCCV_CV); /* Set phase CV */
//  DACC_SetPercentOfRange(percentage);
//}
//
//void VoltageSetter_Plus(uint16_t value)
//{
//  Control_SetCCCV(Control_CCCV_CV); /* Set phase CV */
//  DACC_Plus(value);
//}
//
//void VoltageSetter_Minus(uint16_t value)
//{
//  Control_SetCCCV(Control_CCCV_CV); /* Set phase CV */
//  DACC_Minus(value);
//}
//
//uint32_t VoltageSetter_GetPercentOfRange(uint8_t percentage)
//{
//  uint8_t p;
//  
//  if (percentage >= 100)
//  {
//    p = 100;
//  }
//  else
//  {
//    p = percentage;
//  }
//  
//  switch (Voltmeter_GetRange())
//    {
//      case Voltmeter_HighVoltage:
//        return (VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE / 100) * (uint32_t)p;
//      case Voltmeter_LowVoltage:
//        return (VOLTAGE_SETTER_MAXIMUM_LOVOLTAGE / 100) * (uint32_t)p;
//      default:
//      return 0;
//    }
//}

const ErrorMessaging_Error * VoltageSetter_GetError(void)
{
  return &VoltageSetterError;
}

/* </Implementations> */ 
