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

#define SN                        			"SN155" /* Serial number */
#define CALIBRATION_DATE 					"2022-05-31"

#define CURRENTSETTER_SLOPE_HI 				12543085L
#define CURRENTSETTER_OFFSET_HI 			368834L
#define CURRENTSETTER_SLOPE_LO 				1570581L
#define CURRENTSETTER_OFFSET_LO 			46102L

#define AMMETER_SLOPE_HI 					12543265L
#define AMMETER_OFFSET_HI 					4520L
#define AMMETER_SLOPE_LO 					1570616L
#define AMMETER_OFFSET_LO 					620L

#define VOLTSETTER_SLOPE_HI 				32033416L
#define VOLTSETTER_OFFSET_HI 				952165L
#define VOLTSETTER_SLOPE_LO 				5563999L
#define VOLTSETTER_OFFSET_LO 				164900L

#define VOLTMETER_SLOPE_HI 					32037116L
#define VOLTMETER_OFFSET_HI 				-461L
#define VOLTMETER_SLOPE_LO 					5564451L
#define VOLTMETER_OFFSET_LO 				515L


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
