/**
 * Ammeter.h
 *
 * 2016-10-22
 * kaktus circuits
 * GNU GPL v.3
 */
 
 
#ifndef AMMETER_H
#define AMMETER_H

/* <Includes> */ 

#include "MightyWatt.h"
#include "Configuration.h"
#include "Measurement.h"
#include "ErrorMessaging.h"
 
/* </Includes> */ 
 
 
/* <Defines> */ 
 
#define AMMETER_DEFAULT_MEASUREMENT_SPEED     Measurement_Slow

#define AMMETER_MAXIMUM_CURRENT               (AMMETER_SLOPE_HI + AMMETER_OFFSET_HI) /* Ammeter standard range */
#define AMMETER_MINIMUM_CURRENT               (-100000) /* Negative current */
 
/* </Defines> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes ammeter
 */
void Ammeter_Init(void);

/**
 * Executable function which must be called periodically
 * Calculates new voltage from raw ADC readings
 * Manages hardware ranges (autoranging)
 */
void Ammeter_Do(void);

/**
 * Sets the measurement speed of the ammeter ADC
 *
 * @param msp - one of the pre-defined measurement speed
 */
void Ammeter_SetSpeed(Measurement_Speeds msp);

/**
 * Gets a pointer to the structure containing current from the ammeter
 *
 * @return - see description
 */
const TSCADCULong * Ammeter_GetCurrent(void);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * Ammeter_GetError(void);

/* </Declarations (prototypes)> */

#endif /* AMMETER_H */
