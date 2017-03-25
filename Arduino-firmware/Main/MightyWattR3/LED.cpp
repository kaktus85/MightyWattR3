  /**
 * LED.cpp
 * LED management
 *
 * 2016-10-23
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "LED.h"

/* </Includes> */ 


/* <Module variables> */ 

bool ledIsOn;

/* </Module variables> */ 


/* <Implementations> */ 

void LED_Init(void)
{
  pinMode(LED_PIN, OUTPUT);
  LED_Off();
}

void LED_Set(uint8_t brightness)
{
  if (brightness > 0)
  {  
    analogWrite(LED_PIN, (uint8_t)brightness);
    ledIsOn = true;
  }
  else
  {
    LED_Off();
  }
}

void LED_Off(void)
{
  //digitalWrite(LED_PIN, LOW);
  analogWrite(LED_PIN, 0);
  ledIsOn = false;
}

bool LED_Get(void)
{
  return ledIsOn;
}

/* </Implementations> */ 
