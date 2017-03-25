/**
 * Fan.cpp
 * Fan basic driver
 *
 * 2016-10-23
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "Fan.h"

/* </Includes> */ 


/* <Module variables> */ 

static Fan_States fanState; /* Present fan state */

/* </Module variables> */ 


/* <Implementations> */ 

void Fan_Init(void)
{
  pinMode(FAN_PIN, OUTPUT);
  fanState = FAN_DEFAULT_STATE;
  Fan_Set(fanState);
}

void Fan_Set(Fan_States state)
{
  switch (state)
  {
    case Fan_On:
      fanState = state;
      digitalWrite(FAN_PIN, HIGH);
    break;
    case Fan_Off:
      fanState = state;
      digitalWrite(FAN_PIN, LOW);
    break;
    default:
    break;
  }
}

Fan_States Fan_Get(void)
{
  return fanState;
}

/* </Implementations> */ 
