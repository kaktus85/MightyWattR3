/**
 * AD569xR.cpp 
 * DAC driver
 *
 * 2016-10-22
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "AD569xR.h"
#include <Wire.h>

/* </Includes> */ 


/* <Module variables> */ 

static ErrorMessaging_Error AD569xRError;

/* </Module variables> */ 


/* <Declarations (prototypes)> */ 

/**
 * Sends a 16-bit word to AD569xR
 * 
 * @param data - Payload
 */
void AD569xR_Send(uint8_t command, uint16_t data);

/* </Declarations (prototypes)> */ 


/* <Implementations> */ 

void AD569xR_Init(void)
{
  AD569xRError.errorCounter = 0;
  AD569xRError.error = ErrorMessaging_AD569xR_Overload;
  
  AD569xR_Send(AD569xR_WRITE_CONTROL_REGISTER, AD569xR_RESET);  
}

bool AD569xR_Set(uint16_t value)
{
  if (value <= AD569xR_MAXIMUM_VALUE)
  {
    AD569xR_Send(AD569xR_WRITE_DAC_AND_INPUT_REGISTERS, value); 
    return true;
  }
  else
  {
    AD569xR_Send(AD569xR_WRITE_DAC_AND_INPUT_REGISTERS, AD569xR_MAXIMUM_VALUE); 
    AD569xRError.errorCounter++;
    AD569xRError.error = ErrorMessaging_AD569xR_Overload;
    return false;
  }
}

void AD569xR_Send(uint8_t command, uint16_t data)
{
  Wire.beginTransmission(AD569xR_ADDRESS);  
  Wire.write(command & 0xFF);
  Wire.write((data >> 8) & 0xFF); /* MSB first */
  Wire.write(data & 0xFF);
  Wire.endTransmission();
}

const ErrorMessaging_Error * AD569xR_GetError(void)
{
  return &AD569xRError;
}

/* </Implementations> */ 
