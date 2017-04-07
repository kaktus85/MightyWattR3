/**
 * Flashreader.cpp
 *
 * 2017-04-07
 * kaktus circuits
 * GNU GPL v.3
 */
 
/* <Includes> */ 

#include "Arduino.h"
#include "MightyWatt.h"

/* </Includes> */ 


/* <Implementations> */ 

void Flashreader_Read(uint8_t* to_ptr, const uint8_t* from_ptr, uint8_t array_length)
{
  if ((to_ptr == NULL) || (from_ptr == NULL)) /* Null pointer check */
  {
    return;
  }
  
  for (uint16_t i = 0; i < array_length; i++)
  {
    #ifdef UNO
      to_ptr[i] = pgm_read_byte(&(from_ptr[i]));
    #elif defined(ZERO)
      to_ptr[i] = from_ptr[i];
    #else
      return;
    #endif
  }
}

/* </Implementations> */ 
