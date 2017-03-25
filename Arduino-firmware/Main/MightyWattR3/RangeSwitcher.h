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

#define FORCE_RANGE_SWITCH_TIMEOUT            500 /* After forced range switch, the range can go down after this amount of milliseconds */

#define CURRENT_GAIN_PIN                      4
#define VOLTAGE_GAIN_PIN                      9
#define CURRENT_DEFAULT_HARDWARE_RANGE        CurrentRange_LowCurrent
#define VOLTAGE_DEFAULT_HARDWARE_RANGE        VoltageRange_HighVoltage

#define AMMETER_HYSTERESIS_UP                 (AMMETER_SLOPE_LO * 24 / 25 + AMMETER_OFFSET_LO) /* if over 96 %, go up */
#define AMMETER_HYSTERESIS_DOWN               (AMMETER_SLOPE_LO * 9 / 10 + AMMETER_OFFSET_LO) /* if below 90 %, go down */
#define CURRENTSETTER_HYSTERESIS_UP           (CURRENTSETTER_SLOPE_LO * 24 / 25 - CURRENTSETTER_OFFSET_LO) /* if over 96 %, go up */
#define CURRENTSETTER_HYSTERESIS_DOWN         (CURRENTSETTER_SLOPE_LO * 9 / 10 - CURRENTSETTER_OFFSET_LO) /* if below 90 %, go down */

#define VOLTMETER_HYSTERESIS_UP               (VOLTMETER_SLOPE_LO * 24 / 25 + VOLTMETER_OFFSET_LO) /* if over 96 %, go up */
#define VOLTMETER_HYSTERESIS_DOWN             (VOLTMETER_SLOPE_LO * 9 / 10 + VOLTMETER_OFFSET_LO) /* if below 90 %, go down */
#define VOLTAGESETTER_HYSTERESIS_UP           (VOLTSETTER_SLOPE_LO * 24 / 25 - VOLTSETTER_OFFSET_LO) /* if over 96 %, go up */
#define VOLTAGESETTER_HYSTERESIS_DOWN         (VOLTSETTER_SLOPE_LO * 9 / 10 - VOLTSETTER_OFFSET_LO) /* if below 90 %, go down */

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
 * Call periodically
 * Manages hardware ranges (autoranging)
 */
void RangeSwitcher_Do(void);

/**
 * Gets the hardware range of the ammeter / current setter
 *
 * @return - the current range setting
 */
RangeSwitcher_CurrentRanges RangeSwitcher_GetCurrentRange(void);

/**
 * Immediately increases the range of the ammeter / current setter
 */
void RangeSwitcher_CurrentRangeUp(void);

/**
 * Gets the hardware range of the voltmeter / voltage setter
 *
 * @return - the voltage range setting
 */
RangeSwitcher_VoltageRanges RangeSwitcher_GetVoltageRange(void);

/**
 * Immediately increases the range of the voltmeter / voltage setter
 */
void RangeSwitcher_VoltageRangeUp(void);

/* </Declarations (prototypes)> */ 


#endif /* RANGESWITCHER_H */
