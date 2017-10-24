/**
 * PinController.h
 * Manages communication requests for uncommited pins
 *
 * 2017-10-24
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef PINCONTROLLER_H
#define PINCONTROLLER_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes the module
 */
void PinController_Init(void);

/**
 * Executable function which must be called periodically
 */
void PinController_Do(void);

/**
 * Gets the logical status of pins
 *
 * @return - Flag word with pin statuses arranged as logical pins (sequential from 0)
 */
uint8_t PinController_GetPins(void);

/* </Declarations (prototypes)> */ 

#endif /* PINCONTROLLER_H */

