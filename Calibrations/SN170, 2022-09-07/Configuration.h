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

#define SN                        			"SN170" /* Serial number */
#define CALIBRATION_DATE 					"2022-09-07"

#define CURRENTSETTER_SLOPE_HI 				2491688L
#define CURRENTSETTER_OFFSET_HI 			73688L
#define CURRENTSETTER_SLOPE_LO 				312275L
#define CURRENTSETTER_OFFSET_LO 			9299L

#define AMMETER_SLOPE_HI 					2490980L
#define AMMETER_OFFSET_HI 					1600L
#define AMMETER_SLOPE_LO 					312189L
#define AMMETER_OFFSET_LO 					130L

#define VOLTSETTER_SLOPE_HI 				41676795L
#define VOLTSETTER_OFFSET_HI 				1257208L
#define VOLTSETTER_SLOPE_LO 				6055678L
#define VOLTSETTER_OFFSET_LO 				182430L

#define VOLTMETER_SLOPE_HI 					41672127L
#define VOLTMETER_OFFSET_HI 				-1294L
#define VOLTMETER_SLOPE_LO 					6054548L
#define VOLTMETER_OFFSET_LO 				196L


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
