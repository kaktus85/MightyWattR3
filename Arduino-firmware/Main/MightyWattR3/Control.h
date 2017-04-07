/**
 * Control.h
 *
 * 2016-10-25
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef CONTROL_H
#define CONTROL_H


/* <Includes> */ 

#include "MightyWatt.h"
#include "ErrorMessaging.h"

/* </Includes> */ 


/* <Defines> */ 

#define CONTROL_CCCV_PIN                   12
#define CONTROL_CCCV_PIN_DEFAULT_STATE     CCCV_CC
#define CONTROL_BANDWIDTH_LIMIT_CC         2 /* ms */
#define CONTROL_BANDWIDTH_LIMIT_CV         20 /* ms */

#define CONTROL_MAXIMUM_HI_CURRENT_STEP    ((uint32_t)(CURRENTSETTER_SLOPE_HI / (uint32_t)16)) /* 1/16 of the range */
#define CONTROL_MAXIMUM_LO_CURRENT_STEP    ((uint32_t)(CURRENTSETTER_SLOPE_LO / (uint32_t)16)) /* 1/16 of the range */
#define CONTROL_MINIMUM_HI_CURRENT_STEP    ((uint32_t)(CURRENTSETTER_SLOPE_HI / (uint32_t)DAC_MAXIMUM + 1)) /* microamps */
#define CONTROL_MINIMUM_LO_CURRENT_STEP    ((uint32_t)(CURRENTSETTER_SLOPE_LO / (uint32_t)DAC_MAXIMUM + 1)) /* microamps */

#define CONTROL_MAXIMUM_HI_VOLTAGE_STEP    ((uint32_t)(VOLTSETTER_SLOPE_HI / (uint32_t)16)) /* 1/16 of the range */
#define CONTROL_MAXIMUM_LO_VOLTAGE_STEP    ((uint32_t)(VOLTSETTER_SLOPE_LO / (uint32_t)16)) /* 1/16 of the range */
#define CONTROL_MINIMUM_HI_VOLTAGE_STEP    ((uint32_t)(VOLTSETTER_SLOPE_HI / (uint32_t)DAC_MAXIMUM + 1)) /* microvolts */
#define CONTROL_MINIMUM_LO_VOLTAGE_STEP    ((uint32_t)(VOLTSETTER_SLOPE_LO / (uint32_t)DAC_MAXIMUM + 1)) /* microvolts */

/* </Defines> */ 


/* <Enums> */ 

/*
 * Pre-defined CC/CV states
 */
enum Control_CCCVStates : uint8_t
{
  Control_CCCV_CC,
  Control_CCCV_CV
};

enum Control_CurrentActions : uint8_t
{
  Control_CurrentDown,
  Control_CurrentUp
};

enum Control_VoltageActions : uint8_t
{
  Control_VoltageDown,
  Control_VoltageUp
};

/* </Enums> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes the module
 */
void Control_Init(void);

/**
 * Executable function which must be called periodically
 */
void Control_Do(void);

/**
 * Sets the load to CC mode with zero current
 */
void Control_StopLoad(void);

/**
 * Sets the desired phase for the op-amp that keeps constant values. 
 * Current and voltage have opposing phases for control and must be set according to the mode of the load.
 *
 * @param state - enumeration of the CC/CV state
 */
void Control_SetCCCV(Control_CCCVStates state);

/**
 * Returns CC/CV state
 *
 * @return - CC or CV state
 */
Control_CCCVStates Control_GetCCCV(void);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * Control_GetError(void);

/* </Declarations (prototypes)> */ 

#endif /* CONTROL_H */
