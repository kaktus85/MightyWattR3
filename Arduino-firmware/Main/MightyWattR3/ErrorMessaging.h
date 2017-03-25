/**
 * ErrorMessaging.h
 *
 * 2016-11-04
 * kaktus circuits
 * GNU GPL v.3
 */

#ifndef ERRORMESSAGING_H
#define ERRORMESSAGING_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 


/* <Enums> */

/**
 * Errors that can happen across the modules
 */
enum ErrorMessaging_Errors : uint8_t
{
  ErrorMessaging_ADC_Overload,				/* critical */
  ErrorMessaging_ADC_NotResponding,			/* critical */
  ErrorMessaging_ADS1x15_ResultNotReady,		/* information */
  ErrorMessaging_AD569xR_Overload,			/* warning */
  ErrorMessaging_Ammeter_CurrentOverload,		/* critical */
  ErrorMessaging_Ammeter_NegativeCurrent,		/* critical */
  ErrorMessaging_Communication_CommandTimeout,		/* warning */  
  ErrorMessaging_CurrentSetter_SetCurrentOverload,	/* warning */  
  
  ErrorMessaging_DACC_Overload,				/* warning */
  ErrorMessaging_DACC_UpperLimitReached,		/* information */
  ErrorMessaging_DACC_LowerLimitReached,		/* information */
  ErrorMessaging_Limiter_CurrentOverload,		/* critical */
  ErrorMessaging_Limiter_VoltageOverload,		/* critical */
  ErrorMessaging_Limiter_PowerOverload,			/* critical */
  ErrorMessaging_Limiter_SOAExceeded,     /* critical */
  ErrorMessaging_Limiter_Overheat,			/* critical */
  
  ErrorMessaging_Limiter_HardwareFault,    /* critical */  
  ErrorMessaging_Measurement_Invalid,			/* critical */
  ErrorMessaging_Thermometer_HardwareFault,		/* critical */
  ErrorMessaging_VoltageSetter_SetVoltageOverload,	/* warning */
  ErrorMessaging_Voltmeter_VoltageOverload,		/* critical */
  ErrorMessaging_Voltmeter_NegativeVoltage		/* critical */
};

/* </Enums> */


/* <Structs> */

/**
 * Structure for generic error messaging
 */
struct ErrorMessaging_Error
{
  uint8_t errorCounter; /* "Unique" number of the error for identification. Int/entional wraparound. Useful for identification if the error has been processed. */
  ErrorMessaging_Errors error; /* Number indicating what error has occured */
};

/* </Structs> */


/* <Exported variables> */

/**
 * Pointer to array of error names
 */
extern const char * ErrorMessaging_ErrorNames[];

/* </Exported variables> */


/* <Declarations (prototypes)> */

/**
 * Initializes the module
 */
void ErrorMessaging_Init(void);

/**
 * Gets a flag word indicating which errors are active
 *
 * @return - flag word with active errors
 */
uint32_t ErrorMessaging_GetErrorFlags(void);

/**
 * Gets the number of possible errors
 *
 * @return - the number of unique errors
 */
uint8_t ErrorMessaging_ErrorNamesCount(void);

/**
 * Gets an error message string description
 *
 * @param errorNumber - the number of unique error
 * @param message - pointer to char array where the message will be copied to
 */
void ErrorMessaging_GetError(uint8_t errorNumber, char * message);

/* </Declarations (prototypes)> */

#endif /* ERRORMESSAGING_H */
