/**
 * Configuration.h - Arduino setting and calibration constants
 *
 * 2017-10-24
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/** 
 * UNCOMMENT THE LINE THAT BELONGS TO YOUR ARDUINO VERSION, COMMENT THE OTHER LINE
 * If you are using Arduino Zero (M0/M0 Pro), the sketch was tested with Arduino.org board.
 */
 
//#define ZERO /* Arduino Zero, M0 and M0 Pro (Cortex M0+, 32-bit) via Native port (USB) */
#define UNO /* Arduino Uno (UNO, 8-bit) */


/* Serial port */

#if (defined(ZERO) && defined(UNO))
  #error Only one type of board can be defined
#elif defined(ZERO)
  #define SerialPort SerialUSB
#elif defined(UNO)
  #define SerialPort Serial
#else
  #error No board defined
#endif


/* Calibration */

#define SN                        			"SN132" /* Serial number */
#define CALIBRATION_DATE 					"2022-02-12"

#define CURRENTSETTER_SLOPE_HI 				11151908L
#define CURRENTSETTER_OFFSET_HI 			330482L
#define CURRENTSETTER_SLOPE_LO 				1396497L
#define CURRENTSETTER_OFFSET_LO 			41284L

#define AMMETER_SLOPE_HI 					11148042L
#define AMMETER_OFFSET_HI 					4581L
#define AMMETER_SLOPE_LO 					1396022L
#define AMMETER_OFFSET_LO 					696L

#define VOLTSETTER_SLOPE_HI 				32053317L
#define VOLTSETTER_OFFSET_HI 				963249L
#define VOLTSETTER_SLOPE_LO 				5569406L
#define VOLTSETTER_OFFSET_LO 				168754L

#define VOLTMETER_SLOPE_HI 					32044867L
#define VOLTMETER_OFFSET_HI 				-1121L
#define VOLTMETER_SLOPE_LO 					5567990L
#define VOLTMETER_OFFSET_LO 				-1650L


/* MightyWatt R3 parameters */

#define BOARD_REVISION                   	"3.1.0"
#define VOLTMETER_INPUT_RESISTANCE       	1000000000UL
#define MAXIMUM_POWER                    	80000000UL

/** 
 * UNCOMMENT THE LINE THAT BELONGS TO YOUR ADC TYPE, COMMENT THE OTHER LINE
 */
 
//#define ADC_TYPE_ADS1015					        /* 12-bit */
#define ADC_TYPE_ADS1115					        /* 16-bit */

#if !defined(ADC_TYPE_ADS1015) && !defined(ADC_TYPE_ADS1115)
  #error No ADC defined
#elif (defined(ADC_TYPE_ADS1015) && defined(ADC_TYPE_ADS1115))
  #error Only one type of ADC can be defined
#endif

#endif /* CONFIGURATION_H */
