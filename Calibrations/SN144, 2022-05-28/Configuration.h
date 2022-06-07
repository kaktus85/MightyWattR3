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

#define SN                        			"SN144" /* Serial number */
#define CALIBRATION_DATE 					"2022-05-28"

#define CURRENTSETTER_SLOPE_HI 				12456574L
#define CURRENTSETTER_OFFSET_HI 			365461L
#define CURRENTSETTER_SLOPE_LO 				1559712L
#define CURRENTSETTER_OFFSET_LO 			45684L

#define AMMETER_SLOPE_HI 					12458075L
#define AMMETER_OFFSET_HI 					4107L
#define AMMETER_SLOPE_LO 					1559892L
#define AMMETER_OFFSET_LO 					580L

#define VOLTSETTER_SLOPE_HI 				32038493L
#define VOLTSETTER_OFFSET_HI 				949559L
#define VOLTSETTER_SLOPE_LO 				5565243L
#define VOLTSETTER_OFFSET_LO 				165063L

#define VOLTMETER_SLOPE_HI 					32044696L
#define VOLTMETER_OFFSET_HI 				-209L
#define VOLTMETER_SLOPE_LO 					5566193L
#define VOLTMETER_OFFSET_LO 				-77L


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
