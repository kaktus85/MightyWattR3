/**
 * ADS1x15.h
 *
 * 2016-11-20
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef ADS1x15_H
#define ADS1x15_H

/* <Includes> */ 
 
//#include "MightyWatt.h"
//#include "ErrorMessaging.h"
#include <stdint.h> 
#include "Configuration.h"
 
/* </Includes> */ 


/* <Defines> */ 

#define ADS1x15_OVERRANGE           31130UL
#define ADS1x15_UNDERRANGE          13107UL

#define ADS1x15_ADDRESS             0b01001000
#define ADS1x15_READY_PIN           11

#define ADS1x15_HI_THRESH           0x8000
#define ADS1x15_LO_THRESH           0

/* Data rates */
#ifdef ADC_TYPE_ADS1x15
  #define ADS1x15_DataRates          ADS1x15_DataRates
#elif defined(ADC_TYPE_ADS1115)
  #define ADS1x15_DataRates          ADS1115_DataRates
#else
  #error No ADC defined
#endif

/* config register */
#define ADS1x15_OS_BEGIN_CONVERSION (1 << 15)
#define ADS1x15_MODE_SINGLE_SHOT    (1 << 8)
#define ADS1x15_COMP_MODE_WINDOW    (1 << 4)
#define ADS1x15_COMP_POL_HIGH       (1 << 3)
#define ADS1x15_COMP_LAT_LATCHING   (1 << 2)
#define ADS1x15_COMP_QUE_ONE        0
#define ADS1x15_COMP_QUE_TWO        0b01
#define ADS1x15_COMP_QUE_FOUR       0b10
#define ADS1x15_COMP_DISABLE        0b11

/* </Defines> */ 


/* <Enums> */ 

/*
 * Internal registers of the ADS1x15
 */ 
enum ADS1x15_Registers : uint8_t
{
  ADS1x15_ConversionRegister = 0,
  ADS1x15_ConfigRegister = 1,
  ADS1x15_LoThresholdRegister = 2,
  ADS1x15_HiThresholdRegister = 3  
};

/**
 * Multiplexers (single-ended)
 */
enum ADS1x15_Inputs : uint16_t
{
  ADS1x15_AIN0AIN1 = (0b000 << 12),
  ADS1x15_AIN0AIN3 = (0b001 << 12),
  ADS1x15_AIN1AIN3 = (0b010 << 12),
  ADS1x15_AIN2AIN3 = (0b011 << 12),
  ADS1x15_AIN0GND = (0b100 << 12),
  ADS1x15_AIN1GND = (0b101 << 12),
  ADS1x15_AIN2GND = (0b110 << 12),
  ADS1x15_AIN3GND = (0b111 << 12)
};

/**
 * Voltage ranges (+/- millivolts)
 */
enum ADS1x15_Ranges : uint16_t
{
  ADS1x15_PGA6144 = (0),
  ADS1x15_PGA4096 = (0b001 << 9),
  ADS1x15_PGA2048 = (0b010 << 9),
  ADS1x15_PGA1024 = (0b011 << 9),
  ADS1x15_PGA512 = (0b100 << 9),
  ADS1x15_PGA256 = (0b101 << 9)
};

/**
 * Data rates (samples per second)
 */
enum ADS1015_DataRates : uint16_t
{
  ADS1015_128SPS = (0),
  ADS1015_250SPS = (0b001 << 5),
  ADS1015_490SPS = (0b010 << 5),
  ADS1015_920SPS = (0b011 << 5),
  ADS1015_1600SPS = (0b100 << 5),
  ADS1015_2400SPS = (0b101 << 5),
  ADS1015_3300SPS = (0b110 << 5),
};

/**
 * Data rates (samples per second)
 */
enum ADS1115_DataRates : uint16_t
{
  ADS1115_8SPS = (0),
  ADS1115_16SPS = (0b001 << 5),
  ADS1115_32SPS = (0b010 << 5),
  ADS1115_64SPS = (0b011 << 5),
  ADS1115_128SPS = (0b100 << 5),
  ADS1115_250SPS = (0b101 << 5),
  ADS1115_475SPS = (0b110 << 5),
  ADS1115_860SPS = (0b111 << 5),
};

/* </Enums> */ 


/* <Structs> */ 

/**
 * Working data (settings) for ADC channel
 */
struct ADS1x15_ChannelSetting
{
  ADS1x15_Inputs input;
  ADS1x15_Ranges range;
  ADS1x15_DataRates dataRate;
  bool autorange;
};

/* </Structs> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes ADS1x15
 */
void ADS1x15_Init(void);

/**
 * Start a single conversion 
 *
 * @param channelSetting - structure with input, range and dataRate
 */
void ADS1x15_StartConversion(ADS1x15_ChannelSetting channelSetting);

/**
 * Returns whether conversion is ready and can be read
 *
 * @return - True if result is ready, false otherwise
 */
bool ADS1x15_ConversionReady(void);

/**
 * Returns the raw read value from the ADC
 *
 * @return - Raw reading from the ADC
 */
int16_t ADS1x15_GetRawResult(void);

/**
 * Calculates the voltage from raw result and voltage range
 *
 * @param rawResult - Result read from the ADC
 * @param range - The voltage range that corresponds to the raw result
 *
 * @return - Voltage in 1 LSB = 7.8125e-6 V
 */
int32_t ADS1x15_Voltage(int16_t rawResult, ADS1x15_Ranges range);

/**
 * Calculates new voltage range based on measured raw value and present range
 *
 * @param rawResult - Result read from the ADC
 * @param *range - Pointer to the present range which may be changed by a more suitable one
 */
void ADS1x15_AutoRange(int16_t rawResult, ADS1x15_Ranges * range);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
//const ErrorMessaging_Error * ADS1x15_GetError(void);

/* </Declarations (prototypes)> */ 

#endif /* ADS1x15_H */

