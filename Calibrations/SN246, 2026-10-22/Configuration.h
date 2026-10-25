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

#define SN                        			"SN246" /* Serial number */
#define CALIBRATION_DATE 					"2026-10-22"

#define CURRENTSETTER_SLOPE_HI 				1999588L
#define CURRENTSETTER_OFFSET_HI 			58280L
#define CURRENTSETTER_SLOPE_LO 				250488L
#define CURRENTSETTER_OFFSET_LO 			7289L

#define AMMETER_SLOPE_HI 					1999489L
#define AMMETER_OFFSET_HI 					1025L
#define AMMETER_SLOPE_LO 					250479L
#define AMMETER_OFFSET_LO 					136L

#define VOLTSETTER_SLOPE_HI 				52730167L
#define VOLTSETTER_OFFSET_HI 				1560086L
#define VOLTSETTER_SLOPE_LO 				5927074L
#define VOLTSETTER_OFFSET_LO 				176170L

#define VOLTMETER_SLOPE_HI 					52735821L
#define VOLTMETER_OFFSET_HI 				-1287L
#define VOLTMETER_SLOPE_LO 					5927186L
#define VOLTMETER_OFFSET_LO 				-784L


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
