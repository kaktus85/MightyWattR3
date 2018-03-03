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

#define SN                        			"SN036" /* Serial number */
#define CALIBRATION_DATE 					"2018-03-03"

#define CURRENTSETTER_SLOPE_HI 				11124018L
#define CURRENTSETTER_OFFSET_HI 			332890L
#define CURRENTSETTER_SLOPE_LO 				1390123L
#define CURRENTSETTER_OFFSET_LO 			40954L

#define AMMETER_SLOPE_HI 					11119592L
#define AMMETER_OFFSET_HI 					2814L
#define AMMETER_SLOPE_LO 					1390012L
#define AMMETER_OFFSET_LO 					663L

#define VOLTSETTER_SLOPE_HI 				32016808L
#define VOLTSETTER_OFFSET_HI 				953990L
#define VOLTSETTER_SLOPE_LO 				5560132L
#define VOLTSETTER_OFFSET_LO 				166251L

#define VOLTMETER_SLOPE_HI 					32018643L
#define VOLTMETER_OFFSET_HI 				-607L
#define VOLTMETER_SLOPE_LO 					5563235L
#define VOLTMETER_OFFSET_LO 				-2508L


/* MightyWatt R3 parameters */

#define BOARD_REVISION                   	"3.1.0"
#define VOLTMETER_INPUT_RESISTANCE       	1000000000UL
#define MAXIMUM_POWER                    	75000000UL

/** 
 * UNCOMMENT THE LINE THAT BELONGS TO YOUR ADC TYPE, COMMENT THE OTHER LINE
 */
 
#define ADC_TYPE_ADS1015					        /* 12-bit */
//#define ADC_TYPE_ADS1115					        /* 16-bit */

#if !defined(ADC_TYPE_ADS1015) && !defined(ADC_TYPE_ADS1115)
  #error No ADC defined
#elif (defined(ADC_TYPE_ADS1015) && defined(ADC_TYPE_ADS1115))
  #error Only one type of ADC can be defined
#endif

#endif /* CONFIGURATION_H */
