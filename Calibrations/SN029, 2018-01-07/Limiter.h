/**
 * Limiter.h
 *
 * 2016-11-02
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef LIMITER_H
#define LIMITER_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Defines> */ 

#define LIMITER_MAXIMUM_TEMPERATURE              110
#define LIMITER_MAXIMUM_SOA                      625000000UL /* safe operating area for the transistor - uW */

#define LIMITER_VOLTAGE_ALLOWED_OVERLOAD         50000 /* 100 mV.ms */
#define LIMITER_CURRENT_ALLOWED_OVERLOAD         50000 /* 100 uC */
#define LIMITER_POWER_ALLOWED_OVERLOAD           6600000 /* 100 uJ */

/* </Defines> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes the module
 */
void Limiter_Init(void);

/**
 * Executable function which must be called periodically
 */
void Limiter_Do(void);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * Limiter_GetError(void);

/* </Declarations (prototypes)> */ 

#endif /* LIMITER_H */
