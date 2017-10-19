/**
 * Configuration.h - Arduino setting and calibration constants
 *
 * 2017-04-04
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

#define CALIBRATION_DATE 					"2017-10-19"

#define CURRENTSETTER_SLOPE_HI 				2558019L
#define CURRENTSETTER_OFFSET_HI 			75721L
#define CURRENTSETTER_SLOPE_LO 				319901L
#define CURRENTSETTER_OFFSET_LO 			9403L

#define AMMETER_SLOPE_HI 					2558425L
#define AMMETER_OFFSET_HI 					158L
#define AMMETER_SLOPE_LO 					319948L
#define AMMETER_OFFSET_LO 					89L

#define VOLTSETTER_SLOPE_HI 				32139122L
#define VOLTSETTER_OFFSET_HI 				952655L
#define VOLTSETTER_SLOPE_LO 				5580166L
#define VOLTSETTER_OFFSET_LO 				163815L

#define VOLTMETER_SLOPE_HI 					32146928L
#define VOLTMETER_OFFSET_HI 				179L
#define VOLTMETER_SLOPE_LO 					5581370L
#define VOLTMETER_OFFSET_LO 				1762L


/* MightyWatt R3 parameters */

#define BOARD_REVISION                   	"3.1.0"
#define VOLTMETER_INPUT_RESISTANCE       	1000000000UL
#define MAXIMUM_POWER                    	75000000UL

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
