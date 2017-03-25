/**
 * LEDController.h
 *
 * 2016-10-31
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Defines> */ 

#define LED_CONTROLLER_DEFAULT_RULE             LEDRule_I1
#define LED_CONTROLLER_DEFAULT_BRIGHTNESS       105

/* Define hysteresis for LED rules */
#define LED_CONTROLLER_P1_UP           100 /* reciprocal value */
#define LED_CONTROLLER_P1_DOWN         120 /* reciprocal value */
#define LED_CONTROLLER_V1_UP           100 /* reciprocal value */
#define LED_CONTROLLER_V1_DOWN         120 /* reciprocal value */
#define LED_CONTROLLER_I1_UP           100 /* reciprocal value */
#define LED_CONTROLLER_I1_DOWN         120 /* reciprocal value */
#define LED_CONTROLLER_P10_UP           10 /* reciprocal value */
#define LED_CONTROLLER_P10_DOWN         12 /* reciprocal value */
#define LED_CONTROLLER_V10_UP           10 /* reciprocal value */
#define LED_CONTROLLER_V10_DOWN         12 /* reciprocal value */
#define LED_CONTROLLER_I10_UP           10 /* reciprocal value */
#define LED_CONTROLLER_I10_DOWN         12 /* reciprocal value */
#define LED_CONTROLLER_T50_UP           50
#define LED_CONTROLLER_T50_DOWN         48

/* </Defines> */ 


/* <Enums> */ 

/**
 * Enumeration of states when LED will be active
 * Multiple rules can be active (ORed) at the same time
 * 0 - LED always off
 * 1 - on if P > 1 % of range
 * 2 - on if V > 1 % of range
 * 4 - on if I > 1 % of range
 * 8 - on if P > 10 % of range
 * 16 - on if V > 10 % of range
 * 32 - on if I > 10 % of range
 * 64 - on if temperature > 50°C
 * 128 - LED always on
 */
enum LEDController_Rules : uint8_t
{
  LEDRule_AlwaysOff = 0,
  LEDRule_P1 = 1,
  LEDRule_V1 = 2,
  LEDRule_I1 = 4,
  LEDRule_P10 = 8,
  LEDRule_V10 = 16,
  LEDRule_I10 = 32,
  LEDRule_T50 = 64,
  LEDRule_AlwaysOn = 128
};

/* </Enums> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes the module
 */
void LEDController_Init(void);

/**
 * Executable function which must be called periodically
 */
void LEDController_Do(void);

/* </Declarations (prototypes)> */ 

#endif /* LEDCONTROLLER_H */
