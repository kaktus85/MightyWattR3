/**
 * LED.h
 *
 * 2016-10-23
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef LED_H
#define LED_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Defines> */ 

#define LED_PIN                     3

/* </Defines> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes LED
 */
void LED_Init(void);

/**
 * Sets PWM value to LED
 *
 * @param value - PWM duty cycle value from pre-defined enumeration
 */
void LED_Set(uint8_t brightness);

/**
 * Turns LED off
 */
void LED_Off(void);

/**
 * Gets whether LED is on or off
 *
 * @return - True if LED is on, false if off
 */
bool LED_Get(void);

/* </Declarations (prototypes)> */ 

#endif /* LED_H */
