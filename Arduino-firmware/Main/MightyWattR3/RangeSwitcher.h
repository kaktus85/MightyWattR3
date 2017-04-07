/**
 * RangeSwitcher.h
 *
 * 2017-02-04
 * kaktus circuits
 * GNU GPL v.3
 */
 
 
#ifndef RANGESWITCHER_H
#define RANGESWITCHER_H

/* <Includes> */ 

#include "MightyWatt.h"
#include "Configuration.h"
 
/* </Includes> */ 


/* <Defines> */ 

#define CURRENT_GAIN_PIN                      4
#define VOLTAGE_GAIN_PIN                      9
#define CURRENT_DEFAULT_HARDWARE_RANGE        CurrentRange_LowCurrent
#define VOLTAGE_DEFAULT_HARDWARE_RANGE        VoltageRange_HighVoltage

/* </Defines> */ 


/* <Enums> */ 

/**
  * Hardware ranges of the ammeter / current setter
  */
enum RangeSwitcher_CurrentRanges
{
  CurrentRange_HighCurrent = 0,
  CurrentRange_LowCurrent = 1
};

/**
  * Hardware ranges of the voltmeter / voltage setter
  */
enum RangeSwitcher_VoltageRanges
{
  VoltageRange_HighVoltage = 0,
  VoltageRange_LowVoltage = 1
};

/* </Enums> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes the module
 */
void RangeSwitcher_Init(void);

/**
 * Executable function which must be called periodically
 */
void RangeSwitcher_Do(void);

/**
 * Sets requested current range
 * 
 * @param range - Current range to set
 */
void RangeSwitcher_SetCurrentRange(RangeSwitcher_CurrentRanges range);

/**
 * Sets requested voltage range
 * 
 * @param range - Voltage range to set
 */
void RangeSwitcher_SetVoltageRange(RangeSwitcher_VoltageRanges range);

/**
 * Gets the hardware range of the ammeter / current setter
 *
 * @return - the current range setting
 */
RangeSwitcher_CurrentRanges RangeSwitcher_GetCurrentRange(void);

/**
 * Gets the hardware range of the voltmeter / voltage setter
 *
 * @return - the voltage range setting
 */
RangeSwitcher_VoltageRanges RangeSwitcher_GetVoltageRange(void);

/**
 * Gets whether the current setter should use autoranging
 * If true, it can, if false, the range should be fixed on high range
 * 
 * @return - True if autoranging is allowed by user
 */
bool RangeSwitcher_CanAutorangeCurrent(void);

/**
 * Gets whether the voltage setter should use autoranging
 * If true, it can, if false, the range should be fixed on high range
 * 
 * @return - True if autoranging is allowed by user
 */
bool RangeSwitcher_CanAutorangeVoltage(void);


/* </Declarations (prototypes)> */ 


#endif /* RANGESWITCHER_H */
