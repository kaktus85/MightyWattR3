/**
 * Fan.h
 *
 * 2016-10-23
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef FAN_H
#define FAN_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Defines> */ 

#define FAN_PIN                5
#define FAN_DEFAULT_STATE      Fan_On

/* </Defines> */ 


/* <Enums> */ 

/*
 * Pre-defined fan states
 */
enum Fan_States : uint8_t
{
  Fan_On,
  Fan_Off
};

/* </Enums> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes fan
 */
void Fan_Init(void);

/**
 * Sets fan state
 *
 * @param state - power state of fan
 */
void Fan_Set(Fan_States state);

/**
 * Gets fan state
 *
 * @return - power state of fan
 */
Fan_States Fan_Get(void);

/* </Declarations (prototypes)> */ 

#endif /* FAN_H */
