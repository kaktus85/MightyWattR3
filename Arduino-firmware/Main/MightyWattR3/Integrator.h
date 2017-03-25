/**
 * Integrator.h
 *
 * 2016-11-02
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef INTEGRATOR_H
#define INTEGRATOR_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Enums> */ 

enum Integrator_States
{
  Integrator_None, /* Integral has zero value */
  Integrator_Some, /* Integral has non-zero value but is below overload */
  Integrator_Over  /* Integral is over allowed overload */
};

/* </Enums> */ 


/* <Structs> */ 

struct Integrator_Limits
{
  uint16_t limit;
  uint32_t allowedOverload; /* integral */   
};

/* </Structs> */ 

/* <Declarations (prototypes)> */ 

/**
 * Integrator is a generic function that is usable in monitoring overload states that should be filtered-averaged
 * Values over limit are integrated and when integral reaches maximum allowed overload, "Over" state is set
 * Values below limit are subtracted from the integral until it reaches zero
 *
 * Adds value to integration counter and returns the state of the integration counter
 * 
 * @param value - value to compare with limits, the difference to be added to or subtracted from the integral
 * @param dT - x-axis difference between two samples
 * @param *integral - pointer to the integral value
 * @param *limits - pointer to constant structure with integration limits
 *
 * @return - state of the integrator
 */
Integrator_States Integrator_Add(uint16_t value, uint16_t dT, uint32_t * integral, const Integrator_Limits * limits);

/**
 * Same as standard integrator but for "minimum value" limits, for example minimum power supply voltage
 * Adds value to integration counter and returns the state of the integration counter
 * 
 * @param value - value to compare with limits, the difference to be added to or subtracted from the integral
 * @param dT - x-axis difference between two samples
 * @param *integral - pointer to the integral value
 * @param *limits - pointer to constant structure with integration limits
 *
 * @return - state of the integrator
 */
Integrator_States IntegratorNegative_Add(uint16_t value, uint16_t dT, uint32_t * integral, const Integrator_Limits * limits);

/* </Declarations (prototypes)> */ 

#endif /* INTEGRATOR_H */
