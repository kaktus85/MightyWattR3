/**
 * DACC.h
 *
 * 2016-10-23
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef DACC_H
#define DACC_H

/* <Includes> */ 

#include "MightyWatt.h"
#include "AD569xR.h"
#include "ErrorMessaging.h"

/* </Includes> */ 


/* <Defines> */ 

#define DAC_REFERENCE_VOLTAGE        AD569xR_REFERENCE_VOLTAGE /* mV*/
#define DAC_MAXIMUM                  AD569xR_MAXIMUM_VALUE

/* </Defines> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes DAC
 */
void DACC_Init(void);

/**
 * Sets raw value to the DAC
 *
 * @param value - Raw voltage in LSBs; 1 LSB = DAC_REFERENCE_VOLTAGE/DAC_MAXIMUM
 *
 * @return - true if command succeeded, false otherwise
 */
bool DACC_SetVoltage(uint16_t value);

/**
 * Sets value to the DAC corresponding to a percentage of full-scale range
 *
 * @param percentage - Percentage of full-scale range (0-100)
 *
 * @return - true if command succeeded, false otherwise
 */
bool DACC_SetPercentOfRange(uint8_t percentage);

/**
 * Increases the current DAC value by the specified amount of LSBs
 *
 * @param value - The addition to the current value
 *                If the final value exceeds DAC limits, maximum possible voltage will be set
 *
 * @return - true if command succeeded, false otherwise
 */
bool DACC_Plus(uint16_t value);

/**
 * Decreases the current DAC value by the specified amount of LSBs
 *
 * @param value - The subtraction from the current value
 *                If the final value would be negative, zero voltage will be set
 *
 * @return - true if command succeeded, false otherwise
 */
bool DACC_Minus(uint16_t value);

/**
 * Gets the present DAC value
 *
 * @return - value that is currently set to DAC
 */
uint16_t DACC_GetValue();

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * DACC_GetError(void);

/* </Declarations (prototypes)> */ 

#endif /* DACC_H */
