/**
 * Integrator.cpp
 * Generic integration overload limiter
 *
 * 2016-11-02
 * kaktus circuits
 * GNU GPL v.3
 */
 
 
/* <Includes> */ 

#include "Integrator.h"

/* </Includes> */ 


/* <Implementations> */ 

Integrator_States Integrator_Add(uint16_t value, uint16_t dT, uint32_t * integral, const Integrator_Limits * limits)
{  
  uint32_t newValue;
  if (value > limits->limit) /* Add to the integral */
  {        
    newValue = *integral + ((uint32_t)(value - limits->limit)) * ((uint32_t)dT);
    if (newValue < *integral)
    {
      *integral = 0xFFFFFFFF; /* Set maximum value on overflow */
    }
    else
    {
      *integral = newValue;
    }
  }
  
  if (value < limits->limit) /* Subtract from the integral */
  {
    newValue = *integral - ((uint32_t)(limits->limit - value)) * ((uint32_t)dT);
    if (newValue > *integral)
    {
      *integral = 0; /* Set zero value on overflow */
    }
    else
    {
      *integral = newValue;
    }
  }
  
  /* Evaluate the integral */
  if (*integral == 0)
  {
    return Integrator_None;
  }
  else if (*integral <= limits->allowedOverload)
  {
    return Integrator_Some;
  }
  else
  {
    return Integrator_Over;
  }
}

Integrator_States IntegratorNegative_Add(uint16_t value, uint16_t dT, uint32_t * integral, const Integrator_Limits * limits)
{  
  uint32_t newValue;
  if (value > limits->limit) /* Subtract from the integral */
  {        
    newValue = *integral - ((uint32_t)(value - limits->limit)) * ((uint32_t)dT);
    if (newValue > *integral)
    {
      *integral = 0; /* Set minimum value on overflow */
    }
    else
    {
      *integral = newValue;
    }
  }
  
  if (value < limits->limit) /* Add to the integral */
  {
    newValue = *integral + ((uint32_t)(limits->limit - value)) * ((uint32_t)dT);
    if (newValue < *integral)
    {
      *integral = 0xFFFFFFFF; /* Set maximum value on overflow */
    }
    else
    {
      *integral = newValue;
    }
  }
  
  /* Evaluate the integral */
  if (*integral == 0)
  {
    return Integrator_None;
  }
  else if (*integral <= limits->allowedOverload)
  {
    return Integrator_Some;
  }
  else
  {
    return Integrator_Over;
  }
}

/* </Implementations> */ 
