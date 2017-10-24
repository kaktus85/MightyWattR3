/**
 * Pin.h
 * Uncommited pin management
 *
 * 2017-10-24
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef PIN_H
#define PIN_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes Pin HW module
 */
void Pin_Init(void);

/**
 * Sets an uncommited pin
 *
 * @param pinWord - A flag word containing the statuses of pins
 */
void Pin_Set(uint8_t pinWord);

/**
 * Gets the status flag word of uncommited logical pins
 *
 * @return - Status flag word of uncommited logical pins
 */
uint8_t Pin_Get(void);

/* </Declarations (prototypes)> */ 

#endif /* PIN_H */

