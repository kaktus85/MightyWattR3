/**
 * Flashreader.h
 *
 * 2017-04-07
 * kaktus circuits
 * GNU GPL v.3
 * 
 * Provides a mechanism for reading from flash memory
 */

#ifndef FLASHREADER_H
#define FLASHREADER_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Declarations (prototypes)> */

/**
 * Copies a block of memory from flash to RAM
 *
 * @param to_ptr - Pointer to the destination byte array
 * @param from_ptr - Pointer to the source byte array
 * @param array_length - Number of bytes to copy
 *
 */
void Flashreader_Read(uint8_t* to_ptr, const uint8_t* from_ptr, uint8_t array_length);

/* </Declarations (prototypes)> */

#endif /* FLASHREADER_H */
