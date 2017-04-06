/**
 * CurrentSetter.h
 *
 * 2016-10-29
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef CURRENTSETTER_H
#define CURRENTSETTER_H

/* <Includes> */ 

#include "MightyWatt.h"
#include "ErrorMessaging.h"

/* </Includes> */ 


/* <Defines> */ 

#define CURRENT_SETTER_MAXIMUM_HICURRENT               (CURRENTSETTER_SLOPE_HI - CURRENTSETTER_OFFSET_HI)
#define CURRENT_SETTER_MAXIMUM_LOCURRENT               (CURRENTSETTER_SLOPE_LO - CURRENTSETTER_OFFSET_LO)

#define CURRENTSETTER_HYSTERESIS_UP                    ((CURRENTSETTER_SLOPE_LO * 24) / 25 - CURRENTSETTER_OFFSET_LO) /* if over 96 %, go up */
#define CURRENTSETTER_HYSTERESIS_DOWN                  ((CURRENTSETTER_SLOPE_LO * 9) / 10 - CURRENTSETTER_OFFSET_LO) /* if below 90 %, go down */

/* </Defines> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes current setter
 */
void CurrentSetter_Init(void);

/**
 * Keeps the set current
 */
void CurrentSetter_Do(void);

/**
 * Sets current to DAC
 *
 * @param current - current in uA
 */
void CurrentSetter_SetCurrent(uint32_t current);

/**
 * Immediately sets zero current to DAC
 */
void CurrentSetter_SetZero();

/**
 * Increases the current by the specified value
 *
 * @param value - The addition to the current value
 *                If the final value exceeds DAC limits, maximum possible current will be set
 */
void CurrentSetter_Plus(uint32_t value);

/**
 * Decreases the current by the specified value
 *
 * @param value - The subtraction from the current value
 *                If the final value would be negative, zero current will be set
 */
void CurrentSetter_Minus(uint32_t value);

/**
 * Returns the current that is supposed to be set
 *
 * @return - Current that is set at the moment
 */
uint32_t CurrentSetter_GetCurrent(void);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * CurrentSetter_GetError(void);

/* </Declarations (prototypes)> */ 

#endif /* CURRENTSETTER_H */

