/**
 * AD569xR.h
 *
 * 2016-10-22
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef AD569xR_H
#define AD569xR_H


/* <Includes> */ 

//#include "MightyWatt.h"
//#include "ErrorMessaging.h"
#include <stdint.h> 

/* </Includes> */ 


/* <Defines> */ 

#define AD569xR_ADDRESS                          0b01001100
#define AD569xR_WRITE_DAC_AND_INPUT_REGISTERS    0b00110000
#define AD569xR_WRITE_CONTROL_REGISTER           0b01000000
#define AD569xR_RESET                            0x8000
#define AD569xR_REFERENCE_VOLTAGE                2500L /* mV */
#define AD569xR_MAXIMUM_VALUE                    0xFFFFUL

/* </Defines> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes AD569xR
 * Resets DAC to start from a known state
 */
void AD569xR_Init(void);
 
/**
 * Sets a raw value to the DAC, 1 LSB = AD569xR_REFERENCE_VOLTAGE/(AD569xR_MAXIMUM_VALUE + 1)
 *
 * @param value - Value to set to the DAC
 * 
 * @return - true if command succeeded, false otherwise
 */
bool AD569xR_Set(uint16_t value);
 
/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
//const ErrorMessaging_Error * AD569xR_GetError(void);
 
/* <Declarations (prototypes)> */ 

#endif /* AD569xR_H */

