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

#define SN                        			"SN135" /* Serial number */
#define CALIBRATION_DATE 					"2022-05-26"

#define CURRENTSETTER_SLOPE_HI 				2490917L
#define CURRENTSETTER_OFFSET_HI 			73633L
#define CURRENTSETTER_SLOPE_LO 				311969L
#define CURRENTSETTER_OFFSET_LO 			9307L

#define AMMETER_SLOPE_HI 					2490456L
#define AMMETER_OFFSET_HI 					1321L
#define AMMETER_SLOPE_LO 					311824L
#define AMMETER_OFFSET_LO 					135L

#define VOLTSETTER_SLOPE_HI 				41683742L
#define VOLTSETTER_OFFSET_HI 				1249397L
#define VOLTSETTER_SLOPE_LO 				6054582L
#define VOLTSETTER_OFFSET_LO 				181595L

#define VOLTMETER_SLOPE_HI 					41673575L
#define VOLTMETER_OFFSET_HI 				580L
#define VOLTMETER_SLOPE_LO 					6056718L
#define VOLTMETER_OFFSET_LO 				-1238L


/* MightyWatt R3 parameters */

#define BOARD_REVISION                   	"3.1.0"
#define VOLTMETER_INPUT_RESISTANCE       	1000000000UL
#define MAXIMUM_POWER                    	80000000UL

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
