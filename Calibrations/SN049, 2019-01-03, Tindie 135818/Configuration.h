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

#define SN                        			"SN049" /* Serial number */
#define CALIBRATION_DATE 					"2019-01-03"

#define CURRENTSETTER_SLOPE_HI 				33599285L
#define CURRENTSETTER_OFFSET_HI 			1042085L
#define CURRENTSETTER_SLOPE_LO 				4191374L
#define CURRENTSETTER_OFFSET_LO 			125326L

#define AMMETER_SLOPE_HI 					33561463L
#define AMMETER_OFFSET_HI 					-12920L
#define AMMETER_SLOPE_LO 					4190222L
#define AMMETER_OFFSET_LO 					1100L

#define VOLTSETTER_SLOPE_HI 				22364992L
#define VOLTSETTER_OFFSET_HI 				677512L
#define VOLTSETTER_SLOPE_LO 				5441381L
#define VOLTSETTER_OFFSET_LO 				164392L

#define VOLTMETER_SLOPE_HI 					22353503L
#define VOLTMETER_OFFSET_HI 				103L
#define VOLTMETER_SLOPE_LO 					5439517L
#define VOLTMETER_OFFSET_LO 				15L


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
