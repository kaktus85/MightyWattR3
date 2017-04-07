/**
 * Voltmeter.h
 *
 * 2016-10-21
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef VOLTMETER_H
#define VOLTMETER_H

/* <Includes> */ 

#include "MightyWatt.h"
#include "Configuration.h"
#include "Measurement.h"
#include "ErrorMessaging.h"
 
/* </Includes> */ 
 
/* <Defines> */ 
 
//#define VOLTMETER_GAIN_PIN                      9
#define VOLTMETER_4TERMINAL_PIN                 8
//#define VOLTMETER_DEFAULT_HARDWARE_RANGE        Voltmeter_HighVoltage
#define VOLTMETER_DEFAULT_MEASUREMENT_SPEED     Measurement_Slow
#define VOLTMETER_DEFAULT_MODE                  Voltmeter_2Terminal

#define VOLTMETER_MAXIMUM_VOLTAGE               (VOLTMETER_SLOPE_HI + VOLTMETER_OFFSET_HI)
#define VOLTMETER_MINIMUM_VOLTAGE               (-200000) /* Negative voltage */
#define VOLTMETER_THRESHOLD_VOLTAGE             ((VOLTMETER_SLOPE_HI + VOLTMETER_OFFSET_HI) / 1000)

#define VOLTMETER_HYSTERESIS_UP               (VOLTMETER_SLOPE_LO * 24 / 25 + VOLTMETER_OFFSET_LO) /* if over 96 %, go up */
#define VOLTMETER_HYSTERESIS_DOWN             (VOLTMETER_SLOPE_LO * 9 / 10 + VOLTMETER_OFFSET_LO) /* if below 90 %, go down */
 
/* </Defines> */ 


/* <Enums> */ 

///**
//  * Hardware ranges of the voltmeter
//  */
//enum Voltmeter_Ranges
//{
//  Voltmeter_HighVoltage = 0,
//  Voltmeter_LowVoltage = 1
//};

/**
  * Hardware modes of measurement - 2-terminal or 4-terminal (Kelvin)
  */
enum Voltmeter_Modes
{
  Voltmeter_2Terminal = 0,
  Voltmeter_4Terminal = 1
};

/* </Enums> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes voltmeter
 */
void Voltmeter_Init(void);

/**
 * Executable function which must be called periodically
 * Calculates new voltage from raw ADC readings
 * Manages hardware ranges (autoranging)
 */
void Voltmeter_Do(void);

/**
 * Sets the measurement speed of the voltmeter ADC
 *
 * @param msp - one of the pre-defined measurement speed
 */
void Voltmeter_SetSpeed(Measurement_Speeds msp);

/**
 * Gets the present voltmeter mode (2-terminal or 4-terminal)
 *
 * @return - Present voltmeter mode
 */
Voltmeter_Modes Voltmeter_GetMode(void);

/**
 * Gets a pointer to the structure containing voltage from the voltmeter
 *
 * @return - see description
 */
const TSCADCULong * Voltmeter_GetVoltage(void);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * Voltmeter_GetError(void);

/* </Declarations (prototypes)> */

#endif /* VOLTMETER_H */
