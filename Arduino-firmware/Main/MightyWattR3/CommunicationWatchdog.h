/**
 * CommunicationWatchdog.h
 *
 * 2016-11-05
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef COMMUNICATIONWATCHDOG_H
#define COMMUNICATIONWATCHDOG_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Defines> */ 

#define COMMUNICATION_WATCHDOG_TIMEOUT          6000 /* milliseconds */

/* </Defines> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes serial port
 */
void CommunicationWatchdog_Init(void);

/**
 * Executable function which must be called periodically
 */
void CommunicationWatchdog_Do(void);

/* </Declarations (prototypes)> */ 


#endif /* COMMUNICATIONWATCHDOG_H */
