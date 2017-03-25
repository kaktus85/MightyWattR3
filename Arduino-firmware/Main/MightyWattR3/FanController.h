/**
 * FanController.h
 *
 * 2016-11-01
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef FANCONTROLLER_H
#define FANCONTROLLER_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Defines> */ 

#define FAN_CONTROLLER_DEFAULT_RULE                   FanRule_AlwaysOn
#define FAN_CONTROLLER_RULES_COUNT                    3

/* Define hysteresis for FAN rules */
#define FAN_CONTROLLER_MINIMUM_ONTIME                 30000UL    /* ms */

#define FAN_CONTROLLER_AUTOHIGH_TEMP_UP               50       /* °C */
#define FAN_CONTROLLER_AUTOHIGH_TEMP_DOWN             35       /* °C */
#define FAN_CONTROLLER_AUTOHIGH_P_UP                  10000000 /* uW */
#define FAN_CONTROLLER_AUTOHIGH_P_DOWN                 8000000 /* uW */

#define FAN_CONTROLLER_AUTOLOW_TEMP_UP                75       /* °C */
#define FAN_CONTROLLER_AUTOLOW_TEMP_DOWN              40       /* °C */
#define FAN_CONTROLLER_AUTOLOW_P_UP                   20000000 /* uW */
#define FAN_CONTROLLER_AUTOLOW_P_DOWN                 16000000 /* uW */

/* </Defines> */ 


/* <Enums> */ 

/**
 * Enumeration of states when fan will be active
 * 0 - fan always on 
 * 1 - turn on > 50 °C or average P (non-moving 15-second integral) > 10 W
 *     turn off < 35 °C and average P (non-moving 15-second integral) < 8 W
 * 2 - turn on > 70 °C or average P (non-moving 15-second integral) > 20 W
 *     turn off < 50 °C and average P (non-moving 15-second integral) < 16 W
 */
enum FanController_Rules : uint8_t
{
  FanRule_AlwaysOn = 0,
  FanRule_AutoHigh = 1,
  FanRule_AutoLow = 2
};

/* </Enums> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes the module
 */
void FanController_Init(void);

/**
 * Executable function which must be called periodically
 */
void FanController_Do(void);

/* </Declarations (prototypes)> */ 


#endif /* FANCONTROLLER_H */
