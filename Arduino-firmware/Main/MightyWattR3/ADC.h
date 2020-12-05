/**
 * ADC.h
 *
 * 2016-10-21
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef ADC_H
#define ADC_H
 
/* <Includes> */ 
 
#include "MightyWatt.h"
#include "ADS1x15.h"
#include "Data.h"
#include "ErrorMessaging.h"
#include "Configuration.h"
 
/* </Includes> */ 


/* <Defines> */ 

#define ADC_CHANNEL_COUNT            3
#define ADC_TIMEOUT                  5U /* Wait for conversion ready, ms */
#define ADC_ABSOLUTEMAXIMUM          (ADC_RECIPROCAL_LSB * 3125L) /* 3125 mV */
#define ADC_DEFAULT_RANGE            ADS1x15_PGA4096
#define ADC_RECIPROCAL_LSB           128L /* mV^-1 */
//#define ADC_REFERENCE_VOLTAGE        ADS1x15_REFERENCE_VOLTAGE

/* Mapping of ADC channels to physical channels */
#define ADC_V_CHANNEL                ADS1x15_AIN2AIN3
#define ADC_I_CHANNEL                ADS1x15_AIN1AIN3
#define ADC_T_CHANNEL                ADS1x15_AIN0AIN3

/* ADC triangle filter */
#define ADC_V_CHANNEL_FILTER_SIZE    42
#define ADC_I_CHANNEL_FILTER_SIZE    42
#define ADC_T_CHANNEL_FILTER_SIZE    1

/* ADC cycle skipping */
#define ADC_V_CHANNEL_SKIP_RATIO     0  /* No ADC cycle skipping */
#define ADC_I_CHANNEL_SKIP_RATIO     0  /* No ADC cycle skipping */
#define ADC_T_CHANNEL_SKIP_RATIO     7  /* Measure only ever 2**7 = 128th cycle */

/* </Defines> */ 


/* <Enums> */ 

/**
 * Enumeration of ADC channels
 */
enum ADC_Channels
{
  ADC_V,
  ADC_I,
  ADC_T,
};

/* </Enums> */ 


/* <Structs> */ 

struct ADC_RateRangingFilter
{
  ADS1x15_DataRates dataRate;
  bool autorange;
  bool filter;
};

struct ADC_TriangleFilterData
{
  const uint16_t filterSize;
  int32_t * data;
  int16_t index;
  int32_t sum;
  int32_t triangleSum;
  bool valid;
};

/* </Structs> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes ADC with default values
 * Physically initializes the ADC
 */
void ADC_Init(void);

/**
 * Executable function which must be called periodically
 */
void ADC_Do(void);

/**
 * Set data rate and autoranging for a single channel
 *
 * @param adcChannel - ADC channel to get the voltage from
 * @param rateRangingFilter - data sampling, autoranging on (true) or off (false) and filter use (true)
 */
void ADC_SetupChannel(ADC_Channels adcChannel, ADC_RateRangingFilter rateRangingFilter);

/**
 * Return a constant pointer to the last measured voltage for a given channel
 *
 * @param adcChannel - ADC channel to get the voltage from
 *
 * @return - Pointer to structure containing voltage in LSBs
 */
const TSCADCLong * ADC_GetVoltage(ADC_Channels adcChannel);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * ADC_GetError(ADC_Channels adcChannel);

/**
 * Resets raw voltage filter for given channel
 * Useful when physical range is switched and the values in filter are from an old range
 * 
 * @param adcChannel - ADC channel whose filter should be flushed
 */
void ADC_ResetFilter(ADC_Channels adcChannel);

/* </Declarations (prototypes)> */ 

#endif /* ADC_H */
