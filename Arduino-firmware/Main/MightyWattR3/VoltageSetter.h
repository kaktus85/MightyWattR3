/**
 * VoltageSetter.h
 *
 * 2016-10-29
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef VOLTAGESETTER_H
#define VOLTAGESETTER_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Defines> */ 

#define VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE               (VOLTSETTER_SLOPE_HI - VOLTSETTER_OFFSET_HI)
#define VOLTAGE_SETTER_MAXIMUM_LOVOLTAGE               (VOLTSETTER_SLOPE_LO - VOLTSETTER_OFFSET_LO)
//#define VOLTAGE_SETTER_MAXIMUM_STEP                    16384 /* LSB*/

/* </Defines> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes voltage setter
 */
void VoltageSetter_Init(void);

/**
 * Executable function which must be called periodically
 */
void VoltageSetter_Do(void);

/**
 * Sets voltage to DAC
 *
 * @param voltage - voltage in uV
 */
void VoltageSetter_SetVoltage(uint32_t voltage);

/**
 * Gets the present set voltage
 *
 * @return - Set voltage
 */
uint32_t VoltageSetter_GetVoltage(void);

/**
 * Increases the DAC value by the specified voltage
 *
 * @param value - The addition to the present value
 *                If the final value exceeds DAC limits, maximum possible voltage will be set
 */
void VoltageSetter_Plus(uint32_t value);

/**
 * Decreases the DAC value by the specified voltage
 *
 * @param value - The subtraction from the present value
 *                If the final value would be negative, zero voltage will be set
 */
void VoltageSetter_Minus(uint32_t value);

//
///**
// * Sets maximum voltage at present voltmeter range to DAC
// */
//void VoltageSetter_SetMaximumVoltage(void);
//
///**
// * Sets percentage of full scale voltage
// *
// * @param percentage - Percentage of full-scale range (0-100)
// */
//void VoltageSetter_SetPercentOfRange(uint8_t percentage);
//
///**
// * Increases the DAC value by the specified amount of LSBs
// *
// * @param value - The addition to the present value
// *                If the final value exceeds DAC limits, maximum possible voltage will be set
// */
//void VoltageSetter_Plus(uint16_t value);
//
///**
// * Decreases the DAC value by the specified amount of LSBs
// *
// * @param value - The subtraction from the present value
// *                If the final value would be negative, zero voltage will be set
// */
//void VoltageSetter_Minus(uint16_t value);
//
///**
// * Gets voltage corresponding to given percentage of full scale voltage
// *
// * @param percentage - Percentage of full-scale range (0-100)
// * @return - Voltage corresponding to given percentage of full scale voltage at present voltmeter range setting
// */
//uint32_t VoltageSetter_GetPercentOfRange(uint8_t percentage);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * VoltageSetter_GetError(void);

/* </Declarations (prototypes)> */ 

#endif /* VOLTAGESETTER_H */
