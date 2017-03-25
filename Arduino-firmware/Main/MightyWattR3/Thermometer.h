/**
 * Thermometer.h
 * Temperature monitoring underneath the main FET
 *
 * 2016-10-23
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef THERMOMETER_H
#define THERMOMETER_H

/* <Includes> */ 

#include "MightyWatt.h"
#include "ErrorMessaging.h"

/* </Includes> */ 


/* <Defines> */ 

/*
#define THERMISTOR_BETA                                 3455
#define THERMISTOR_T0                                   298.15
#define THERMISTOR_R0                                   10000 
*/

#define THERMISTOR_R                                    (39000L + 1200L)
#define THERMOMETER_REFERENCE_VOLTAGE_IN_ADC_LSB        (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB)
#define THERMISTOR_EQ_A                                 0.000688216      /* 1/THERMISTOR_T0 - 1/THERMISTOR_BETA * ln(THERMISTOR_R0) */
#define THERMISTOR_EQ_B                                 0.000289436      /* 1/THERMISTOR_BETA */

/* </Defines> */ 


/* <Includes> */ 

#include "Data.h"

/* </Includes> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes thermometer
 */
void Thermometer_Init(void);

/**
 * Executable function which must be called periodically
 * Calculates new temperature from raw ADC readings
 */
void Thermometer_Do(void);

/**
 * Gets a pointer to the structure containing temperature underneath the main FET from the thermometer
 *
 * @return - see description
 */
const TSCUChar * Thermometer_GetTemperature(void);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * Thermometer_GetError(void);

/* </Declarations (prototypes)> */ 
 
#endif /* THERMOMETER_H */
