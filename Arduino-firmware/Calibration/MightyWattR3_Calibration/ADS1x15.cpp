/**
 * ADS1x15.cpp
 * ADC driver
 *
 * 2016-11-20
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "ADS1x15.h"
#include <Wire.h>

/* </Includes> */ 


/* <Module variables> */ 

static bool conversionReady = false;
//static ErrorMessaging_Error ADS1x15Error;

/* </Module variables> */ 


/* <Declarations (prototypes)> */ 

/**
 * Sends a 16-bit word to ADS1x15
 * 
 * @param reg - Register to which to write
 * @param data - Payload
 */
void ADS1x15_Send(ADS1x15_Registers reg, uint16_t data);

/**
 * Reads a 16-bit word from ADS1x15
 * @param reg - Register from which to read 
 * 
 * @return Register value
 */
uint16_t ADS1x15_Read(ADS1x15_Registers reg);

/* </Declarations (prototypes)> */ 


/* <Implementations> */ 

void ADS1x15_Init(void)
{
  pinMode(ADS1x15_READY_PIN, INPUT);
  ADS1x15_Send(ADS1x15_HiThresholdRegister, ADS1x15_HI_THRESH);
  ADS1x15_Send(ADS1x15_LoThresholdRegister, ADS1x15_LO_THRESH);  
//  ADS1x15Error.errorCounter = 0;
//  ADS1x15Error.error = ErrorMessaging_ADS1x15_ResultNotReady;
}

void ADS1x15_StartConversion(ADS1x15_ChannelSetting channelSetting)
{ 
  conversionReady = false;
  ADS1x15_Send(ADS1x15_ConfigRegister, channelSetting.range | channelSetting.input | channelSetting.dataRate | ADS1x15_OS_BEGIN_CONVERSION | ADS1x15_MODE_SINGLE_SHOT | ADS1x15_COMP_LAT_LATCHING | ADS1x15_COMP_MODE_WINDOW);
}

bool ADS1x15_ConversionReady(void)
{
  if (!conversionReady)
  {    
    conversionReady = (digitalRead(ADS1x15_READY_PIN) == LOW);
  }
  return conversionReady;
}

int16_t ADS1x15_GetRawResult(void)
{
  static int16_t rawResult = 0; /* the ADS1x15 is bipolar */
  if (conversionReady) /* update raw result if conversion ready */
  {
    conversionReady = false; 
    rawResult = (int16_t)ADS1x15_Read(ADS1x15_ConversionRegister);
  }
  else
  {
    /* Requested result but ADC is not ready */
//    ADS1x15Error.errorCounter++;
//    ADS1x15Error.error = ErrorMessaging_ADS1x15_ResultNotReady;    
  }
  return rawResult; /* Result is left-aligned */
}

int32_t ADS1x15_Voltage(int16_t rawResult, ADS1x15_Ranges range)
{
  /* 6144 mV range not used in this function */
  int32_t voltage = rawResult;
  switch (range)
  {
    case ADS1x15_PGA4096:
      voltage *= 16;
    break;
    case ADS1x15_PGA2048:
      voltage *= 8;
    break;
    case ADS1x15_PGA1024:
      voltage *= 4;
    break;
    case ADS1x15_PGA512:
      voltage *= 2;
    break;
    default:
    break;
  }  
  return voltage;
}

void ADS1x15_AutoRange(int16_t rawResult, ADS1x15_Ranges * range)
{
  /* Finite state machine */
  /* 6144 mV range not used in this function */
  if ((rawResult > ADS1x15_OVERRANGE) || (rawResult < -ADS1x15_OVERRANGE))
  {
    /* Switch to higher voltage range */
    switch (*range)
    {
      case ADS1x15_PGA2048:
        *range = ADS1x15_PGA4096;
        break;
      case ADS1x15_PGA1024:
        *range = ADS1x15_PGA2048;
        break;
      case ADS1x15_PGA512:
        *range = ADS1x15_PGA1024;
        break;
      case ADS1x15_PGA256:
        *range = ADS1x15_PGA512;
        break;
      default:
        break;
    }
  }
  
  if ((rawResult < ADS1x15_UNDERRANGE) && (rawResult > -ADS1x15_UNDERRANGE))
  {
    /* Switch to lower voltage range */
    switch (*range)
    {
      case ADS1x15_PGA4096:
        *range = ADS1x15_PGA2048;
        break;
      case ADS1x15_PGA2048:
        *range = ADS1x15_PGA1024;
        break;
      case ADS1x15_PGA1024:
        *range = ADS1x15_PGA512;
        break;
      case ADS1x15_PGA512:
        *range = ADS1x15_PGA256;
        break;
      default:
        break;
    }
  }
}

void ADS1x15_Send(ADS1x15_Registers reg, uint16_t data)
{
  Wire.beginTransmission(ADS1x15_ADDRESS);  
  Wire.write(reg & 0xFF);
  Wire.write((data >> 8) & 0xFF); /* MSB first */
  Wire.write(data & 0xFF); 
  Wire.endTransmission();
}

uint16_t ADS1x15_Read(ADS1x15_Registers reg)
{
  /* set read register */
  Wire.beginTransmission(ADS1x15_ADDRESS);
  Wire.write(reg & 0xFF);
  Wire.endTransmission();

  /* read from the register */
  Wire.requestFrom(ADS1x15_ADDRESS, 2);
  while(Wire.available() == 0) {}
  uint16_t value;
  value = (Wire.read() << 8); /* MSB first */
  value |= Wire.read();
  return value;
}

//const ErrorMessaging_Error * ADS1x15_GetError(void)
//{
//  return &ADS1x15Error;
//}

/* </Implementations> */ 

