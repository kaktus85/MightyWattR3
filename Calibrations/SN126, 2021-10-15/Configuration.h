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

#define SN                        			"SN126" /* Serial number */
#define CALIBRATION_DATE 					"2021-10-15"

#define CURRENTSETTER_SLOPE_HI 				33509267L
#define CURRENTSETTER_OFFSET_HI 			1038018L
#define CURRENTSETTER_SLOPE_LO 				4179737L
#define CURRENTSETTER_OFFSET_LO 			124575L

#define AMMETER_SLOPE_HI 					33510800L
#define AMMETER_OFFSET_HI 					-25553L
#define AMMETER_SLOPE_LO 					4180168L
#define AMMETER_OFFSET_LO 					1626L

#define VOLTSETTER_SLOPE_HI 				10500496L
#define VOLTSETTER_OFFSET_HI 				316525L
#define VOLTSETTER_SLOPE_LO 				2218548L
#define VOLTSETTER_OFFSET_LO 				66566L

#define VOLTMETER_SLOPE_HI 					10501925L
#define VOLTMETER_OFFSET_HI 				279L
#define VOLTMETER_SLOPE_LO 					2218843L
#define VOLTMETER_OFFSET_LO 				355L


/* MightyWatt R3 parameters */

#define BOARD_REVISION                   	"3.1.0"
#define VOLTMETER_INPUT_RESISTANCE       	470000000UL
#define MAXIMUM_POWER                    	90000000UL

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
