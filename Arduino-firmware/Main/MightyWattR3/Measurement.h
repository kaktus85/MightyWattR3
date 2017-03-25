/**
 * Measurement.h
 *
 * 2016-10-21
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef MEASUREMENT_H
#define MEASUREMENT_H
 

/* <Includes> */ 
 
#include "MightyWatt.h"
#include "ADS1x15.h"
#include "ADC.h"
#include "ErrorMessaging.h"
 
/* </Includes> */ 
 
 
/* <Defines> */ 

#define MEASUREMENT_SPEEDS_COUNT        3

/* </Defines> */ 


/* <Enums> */ 

/*
 * Pre-defined combinations of rate and autoranging
 */
enum Measurement_Speeds : uint8_t
{
  Measurement_Fast = 0,
  Measurement_Medium = 1,
  Measurement_Slow = 2
};

/* </Enums> */ 


/* <Structs> */ 

/**
 * Timestamped counted measured and calculated values for the main electrical characteristics
 * Timestamp should contain millisecond counter
 * Counter should update every time all values are renewed
 * Voltage in uV
 * Current in uA
 * Power in uW
 * Resistance in mOhm
 */
struct Measurement_Values
{
  uint32_t milliseconds;
  uint8_t counter;
  uint32_t voltage;
  uint32_t current;
  uint32_t power;
  uint32_t resistance;
  
  uint32_t unfilteredVoltage;
  uint32_t unfilteredCurrent;
  uint32_t unfilteredPower;
  uint32_t unfilteredResistance;
};

/* </Structs> */ 


/* <Exported variables> */ 

extern const ADC_RateRangingFilter Measurement_Speed[];

/* </Exported variables> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes measurement unit
 */
void Measurement_Init(void);

/**
 * Executable function which must be called periodically
 * Gathers new voltage and current from voltmeter and ammeter and calculates power and resistance
 */
void Measurement_Do(void);

/**
 * Gets a pointer to the structure containing voltage, current, power and resistance
 *
 * @return - see description
 */
const Measurement_Values * Measurement_GetValues(void);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * Measurement_GetError(void);

/* </Declarations (prototypes)> */ 

#endif /* MEASUREMENT_H */
