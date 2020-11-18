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

#define SN                        			"SN118" /* Serial number */
#define CALIBRATION_DATE 					"2020-11-11"

#define CURRENTSETTER_SLOPE_HI 				1993317L
#define CURRENTSETTER_OFFSET_HI 			59428L
#define CURRENTSETTER_SLOPE_LO 				249495L
#define CURRENTSETTER_OFFSET_LO 			7595L

#define AMMETER_SLOPE_HI 					1993729L
#define AMMETER_OFFSET_HI 					418L
#define AMMETER_SLOPE_LO 					249552L
#define AMMETER_OFFSET_LO 					-108L

#define VOLTSETTER_SLOPE_HI 				52733631L
#define VOLTSETTER_OFFSET_HI 				1581961L
#define VOLTSETTER_SLOPE_LO 				5926786L
#define VOLTSETTER_OFFSET_LO 				177913L

#define VOLTMETER_SLOPE_HI 					52753614L
#define VOLTMETER_OFFSET_HI 				-1886L
#define VOLTMETER_SLOPE_LO 					5928393L
#define VOLTMETER_OFFSET_LO 				-108L


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
