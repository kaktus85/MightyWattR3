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

#define SN                        			"SN175" /* Serial number */
#define CALIBRATION_DATE 					"2022-09-07"

#define CURRENTSETTER_SLOPE_HI 				2496124L
#define CURRENTSETTER_OFFSET_HI 			73600L
#define CURRENTSETTER_SLOPE_LO 				312848L
#define CURRENTSETTER_OFFSET_LO 			9343L

#define AMMETER_SLOPE_HI 					2496110L
#define AMMETER_OFFSET_HI 					1475L
#define AMMETER_SLOPE_LO 					312840L
#define AMMETER_OFFSET_LO 					70L

#define VOLTSETTER_SLOPE_HI 				41649252L
#define VOLTSETTER_OFFSET_HI 				1252201L
#define VOLTSETTER_SLOPE_LO 				6055240L
#define VOLTSETTER_OFFSET_LO 				183159L

#define VOLTMETER_SLOPE_HI 					41650460L
#define VOLTMETER_OFFSET_HI 				84L
#define VOLTMETER_SLOPE_LO 					6055430L
#define VOLTMETER_OFFSET_LO 				-1124L


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
