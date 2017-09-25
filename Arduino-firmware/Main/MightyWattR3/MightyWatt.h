/**
 * MightyWatt.h
 * common includes
 *
 * 2016-11-16
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef MIGHTYWATT_H
#define MIGHTYWATT_H
 
/* <Includes> */ 
 
#include <stdint.h> 
#include "Configuration.h"
 
/* </Includes> */ 


/* <Defines> */ 

#define NAME                       "MightyWatt R3"
#define FIRMWARE_VERSION           "3.1.2"

#ifdef UNO
  #include <avr/pgmspace.h>
  #define FLASHMEMORY PROGMEM
#elif defined(ZERO)
  #define FLASHMEMORY 
#endif

/* </Defines> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes MightyWatt
 */
void MightyWatt_Init(void);

/**
 * Main executable loop
 */
void MightyWatt_Do(void);

/* </Declarations (prototypes)> */ 
 
#endif /* MIGHTYWATT_H */
