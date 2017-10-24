/**
* Pin.cpp
* Uncommited pin management
*
* 2017-10-24
* kaktus circuits
* GNU GPL v.3
*/


/* <Includes> */ 

#include "Arduino.h"
#include "Pin.h"

/* </Includes> */ 


/* <Module variables> */ 

// Mapping of logical pins to physical pins on Arduino
static const uint8_t pinMapping[] = {2, 6, 7, 10, 13};

// Complete status of the pins
static uint8_t statusWord = 0;

/* </Module variables> */ 


/* <Implementations> */ 

void Pin_Init(void)
{
  statusWord = 0;
  for (uint8_t i = 0; i < sizeof(pinMapping) / sizeof(uint8_t); i++)
  {
    pinMode(pinMapping[i], OUTPUT);
    digitalWrite(pinMapping[i], LOW);
  }
}

void Pin_Set(uint8_t pinWord)
{
  statusWord = pinWord & ((1 << (sizeof(pinMapping) / sizeof(uint8_t))) - 1);
  for (uint8_t i = 0; i < sizeof(pinMapping) / sizeof(uint8_t); i++)
  {
    if (((1 << i) & statusWord) > 0)
    {
      digitalWrite(pinMapping[i], HIGH);
    }
    else
    {
      digitalWrite(pinMapping[i], LOW);
    }    
  }
}

uint8_t Pin_Get(void)
{
  return statusWord;
}

/* </Implementations> */ 

