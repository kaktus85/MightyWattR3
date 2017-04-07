/**
 * IMPORTANT!
 * Navigate to Configuration.h and select your Arduino version and ADC type
 */

#include <math.h>
#include <Wire.h>
#include "ADS1x15.h"
#include "AD569xR.h"
#include "Configuration.h"
#include <stdint.h> 
#include <assert.h>

#define FIRMWARE_VERSION                                "3.1.0"

#define DAC_REFERENCE_VOLTAGE                           AD569xR_REFERENCE_VOLTAGE /* mV */
#define DAC_MAXIMUM                                     AD569xR_MAXIMUM_VALUE
#define ADC_RECIPROCAL_LSB                              128UL /* mV^-1 */

#define THERMOMETER_REFERENCE_VOLTAGE_IN_ADC_LSB        (DAC_REFERENCE_VOLTAGE * ADC_RECIPROCAL_LSB)
#define THERMISTOR_R                                    (39000L + 1200L)
#define THERMISTOR_EQ_A                                 0.000688216      /* 1/THERMISTOR_T0 - 1/THERMISTOR_BETA * ln(THERMISTOR_R0) */
#define THERMISTOR_EQ_B                                 0.000289436      /* 1/THERMISTOR_BETA */

#define VOLTMETER_GAIN_PIN                              9
#define VOLTMETER_4TERMINAL_PIN                         8
#define CONTROL_CCCV_PIN                                12
#define AMMETER_GAIN_PIN                                4
#define FAN_PIN                                         5
#define LED_PIN                                         3

#define ADCV_RANGE                                      ADS1x15_PGA4096
#define ADCI_RANGE                                      ADS1x15_PGA4096
#define ADCT_RANGE                                      ADS1x15_PGA4096
#define ADCVos_RANGE                                    ADS1x15_PGA256
#ifdef ADC_TYPE_ADS1015
  #define ADC_DATA_RATE                                 ADS1015_128SPS
  #define ADC_AVERAGING                                 256
#elif defined(ADC_TYPE_ADS1115)
  #define ADC_DATA_RATE                                 ADS1115_8SPS
  #define ADC_AVERAGING                                 16
#else
  #error No ADC type defined
#endif

ADS1x15_ChannelSetting ADCV =
{
  ADS1x15_AIN2AIN3,
  ADCV_RANGE,
  ADC_DATA_RATE,
  false
};

ADS1x15_ChannelSetting ADCI =
{
  ADS1x15_AIN1AIN3,
  ADCI_RANGE,
  ADC_DATA_RATE,
  false
};

ADS1x15_ChannelSetting ADCT =
{
  ADS1x15_AIN0AIN3,
  ADCT_RANGE,
  ADC_DATA_RATE,
  false
};

ADS1x15_ChannelSetting ADCVos =
{
  ADS1x15_AIN3GND,
  ADCVos_RANGE,
  ADC_DATA_RATE,
  false
};

void setup()
{     
  delayMicroseconds(10000); /* delay to give the hardware some time to stabilize, it is usually not needed but just for peace of mindâ€¦ */       
  SerialPort.begin(9600);  
  while(!SerialPort){}; /* Wait for the initialization of serial port */
  while(SerialPort.read() >= 0){}; /* Read all junk data already at the port */ 
  Wire.begin();   
  ADS1x15_Init();
  AD569xR_Init();
  
  pinMode(VOLTMETER_GAIN_PIN, OUTPUT);
  pinMode(VOLTMETER_4TERMINAL_PIN, OUTPUT);
  pinMode(CONTROL_CCCV_PIN, OUTPUT);
  pinMode(AMMETER_GAIN_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  ResetLoad();







  /* Calibration */  
  #define CALIBRATION_POINTS         5 /* Select the number of calibration points */
  /* Select the type of calibration - either current or voltage. Always have one line commented and the other uncommented. */
  #define CALIBRATE_CURRENT       /* Uncomment this line to enable ammeter and constant current calibration. Comment otherwise. */
  //#define CALIBRATE_VOLTAGE       /* Uncomment this line to enable voltmeter and constant voltage calibration. Comment otherwise. */


#if defined(CALIBRATE_CURRENT) && !defined(CALIBRATE_VOLTAGE)

  /* Part 1 - ammeter + constant current mode
   * Current is increased each time something (other than "q") is received on the serial line.
   * You will need to record two set of data - one for the low current range and one for the high current range.
   * Switch current ranges by sending "0" for the high current range and "1" for the low current range when the calibration sequence starts (it will prompt you).
   * 
   * DAC value is written followed by ADC value. Use this in conjunction with values from an external ammeter to calibrate both DAC and ADC for current.
   * Do not overlap ranges; i.e., if you have 3A/24A unit, calibrate the 3A range from 0.03A to 3A and the 24A range from 3A to 24A.
   * 
   * Limit your power supply to low voltage so the load does not have to dissipate excessive power but allow at least 1V drop on the load. Limiting power supply to 2V is a good idea.
   */
       
  #define DAC_PERC_I_HI_RANGE_MAX    99 /* Percent of range, ideally 99% of range.
                                           If your power supply or ammeter does not support such a range, decrease it accordingly */
  #define DAC_PERC_I_HI_RANGE_MIN    12 /* Percent of range, should be around 12% so the ranges do not overlap (significantly) */
 
  #define DAC_PERC_I_LO_RANGE_MAX    99 /* Percent of range */
  #define DAC_PERC_I_LO_RANGE_MIN    4  /* Percent of range, should start from 4% because the DAC has intentional offset */

  // this recalculates percents to DAC scale
  #define DAC_I_HI_MIN                  ((65535/100)*DAC_PERC_I_HI_RANGE_MIN)
  #define DAC_I_HI_MAX                  ((65535/100)*DAC_PERC_I_HI_RANGE_MAX)
  #define DAC_I_LO_MIN                  ((65535/100)*DAC_PERC_I_LO_RANGE_MIN)
  #define DAC_I_LO_MAX                  ((65535/100)*DAC_PERC_I_LO_RANGE_MAX)
  
  assert(DAC_I_HI_MAX > DAC_I_HI_MIN);
  assert(DAC_I_LO_MAX > DAC_I_LO_MIN);

  int32_t adc;
  int command;
  uint16_t dacMin, dacMax;

  SerialPort.println("==============================================");
  SerialPort.println("Ammeter and constant current calibration start");
  SerialPort.println("==============================================");
  SerialPort.println();
  SerialPort.println("Send \"q\" any time to quit");
  SerialPort.println();

  ResetLoad();
  
  do
  {              
    // select range
    SerialPort.println("Select current range");
    SerialPort.println("\"0\" for high current range, \"1\" for low current range, any other character to quit.");
    SerialPort.println();
    
    while(SerialPort.available() == 0){}
    command = SerialPort.read(); // wait for keypress to increase current
    delay(10);
    while(SerialPort.available() > 0) { SerialPort.read(); }
    
    switch (command)
    {
      case '0':
        dacMin = DAC_I_HI_MIN;
        dacMax = DAC_I_HI_MAX;
        digitalWrite(AMMETER_GAIN_PIN, LOW);
      break;
      case '1':
        dacMin = DAC_I_LO_MIN;
        dacMax = DAC_I_LO_MAX;
        digitalWrite(AMMETER_GAIN_PIN, HIGH);
      break;
      default:
      break;
    }
    
    if ((command == '0') || (command == '1'))
    {
      // calibration loop
      SerialPort.println("DAC\tADC");
      for (int32_t i = dacMin; i <= dacMax; i += (dacMax - dacMin) / (CALIBRATION_POINTS - 1) )
      {            
        SerialPort.print((uint16_t)(i & 0xFFFF));
        AD569xR_Set((uint16_t)(i & 0xFFFF)); // set DAC
        delay(2000); // equilibrate
        adc = ADS1x15_Voltage(ADC_ReadRaw(ADCI), ADCI_RANGE); // read ADC
        // print DAC and ADC values
        SerialPort.print("\t");
        SerialPort.println(adc);
        
        while(SerialPort.available() == 0){}
        command = SerialPort.read(); // wait for keypress to increase current    
        delay(10);
        while(SerialPort.available() > 0) { SerialPort.read(); }
        
        if (command == 'q')
        {
          break;
        }
      }
      SerialPort.println();      
    }    
    else
    {
      break; 
    }

    ResetLoad();
  } while(command != 'q');  
  
  SerialPort.println();
  SerialPort.println("============================================");
  SerialPort.println("Ammeter and constant current calibration end");
  SerialPort.println("============================================");
  
  ResetLoad();
  return;

#endif //defined(CALIBRATE_CURRENT) && !defined(CALIBRATE_VOLTAGE)


#if !defined(CALIBRATE_CURRENT) && defined(CALIBRATE_VOLTAGE)

  /* Part 2 - voltmeter + constant voltage mode
   * Voltage is decreased each time something (other than "0" or "1") is received on the serial line.
   * You will need to record two set of data - one for the low voltage range and one for the high voltage range.
   * Switch voltage ranges by sending "0" for the high voltage range and "1" for the low voltage range.
   * 
   * DAC value is written followed by ADC value. Use this in conjunction with external voltmeter to calibrate both DAC and ADC for voltage.
   * Do not overlap ranges; i.e., if you have 5V/30A unit, calibrate the 5V range from 0.05V to 5V and the 30V range from 5V to 30V.
   * 
   * Use 4-wire mode if possible. Limit your power supply to low current so the load does not have to dissipate excessive power.
   */

  
  #define DAC_PERC_V_HI_RANGE_MAX    99 /* Percent of range, ideally 99% of range.
                                           If your power supply or ammeter does not support such a range, decrease it accordingly */
  #define DAC_PERC_V_HI_RANGE_MIN    17 /* Percent of range, should be around 17% so the ranges do not overlap (significantly) */                                           
  
  #define DAC_PERC_V_LO_RANGE_MAX    99 /* Percent of range */
  #define DAC_PERC_V_LO_RANGE_MIN    4  /* Percent of range, should start from 4% because the DAC has intentional offset */

  // this recalculates percents to DAC scale
  #define DAC_V_HI_MIN                  ((65535/100)*DAC_PERC_V_HI_RANGE_MIN)
  #define DAC_V_HI_MAX                  ((65535/100)*DAC_PERC_V_HI_RANGE_MAX)
  #define DAC_V_LO_MIN                  ((65535/100)*DAC_PERC_V_LO_RANGE_MIN)
  #define DAC_V_LO_MAX                  ((65535/100)*DAC_PERC_V_LO_RANGE_MAX)

  assert(DAC_V_HI_MAX > DAC_V_HI_MIN);
  assert(DAC_V_LO_MAX > DAC_V_LO_MIN);

  int32_t adc;
  int command;
  uint16_t dacMin, dacMax;

  SerialPort.println("================================================");
  SerialPort.println("Voltmeter and constant voltage calibration start");
  SerialPort.println("================================================");
  SerialPort.println();
  SerialPort.println("Send \"q\" any time to quit");
  SerialPort.println();
 
  digitalWrite(VOLTMETER_4TERMINAL_PIN, HIGH); // Use 4-wire mode, if not possible, comment out this line

  AD569xR_Set(0xFFFF);
  digitalWrite(CONTROL_CCCV_PIN, HIGH);

  do
  {      
    // select range
    SerialPort.println("Select voltage range");
    SerialPort.println("\"0\" for high voltage range, \"1\" for low voltage range, any other character to quit.");
    SerialPort.println();
    
    while(SerialPort.available() == 0){}
    command = SerialPort.read(); // wait for keypress to increase current
    delay(10);
    while(SerialPort.available() > 0) { SerialPort.read(); }
    
    switch (command)
    {
      case '0':
        dacMin = DAC_V_HI_MIN;
        dacMax = DAC_V_HI_MAX;
        digitalWrite(VOLTMETER_GAIN_PIN, LOW);
      break;
      case '1':
        dacMin = DAC_V_LO_MIN;
        dacMax = DAC_V_LO_MAX;
        digitalWrite(VOLTMETER_GAIN_PIN, HIGH);
      break;
      default:
      break;
    }
    
    if ((command == '0') || (command == '1'))
    {
      // calibration loop
      SerialPort.println("DAC\tADC");
      for (int32_t i = dacMax; i >= dacMin; i -= (dacMax - dacMin) / (CALIBRATION_POINTS - 1))
      {    
        SerialPort.print((uint16_t)(i & 0xFFFF));
        AD569xR_Set((uint16_t)(i & 0xFFFF)); // set DAC
        delay(2000); // equilibrate
        adc = ADS1x15_Voltage(ADC_ReadRaw(ADCV), ADCV_RANGE); // read ADC
        // print DAC and ADC values
        SerialPort.print("\t");
        SerialPort.println(adc);
        
        while(SerialPort.available() == 0){}
        command = SerialPort.read(); // wait for keypress to increase voltage
        delay(10);
        while(SerialPort.available() > 0) { SerialPort.read(); }
        
        if (command == 'q')
        {
          break;
        }
      }
      SerialPort.println();
    }   
    else
    {
      break; 
    }
    
    AD569xR_Set(0xFFFF);
    digitalWrite(CONTROL_CCCV_PIN, HIGH); 
  } while(command != 'q');
  
  SerialPort.println();
  SerialPort.println("==============================================");
  SerialPort.println("Voltmeter and constant voltage calibration end");
  SerialPort.println("==============================================");

  ResetLoad();
  return;

#endif // !defined(CALIBRATE_CURRENT) && defined(CALIBRATE_VOLTAGE)
}

void loop()
{  
  // nothing  
}

/**
 * Performs a series of measurements and returns an averaged value
 * 
 * @param channelSetting - ADC channel settings
 *
 * @return - Averaged raw value from ADC
 */
static int16_t ADC_ReadRaw(ADS1x15_ChannelSetting channelSetting)
{
  int32_t result = 0;
  uint16_t i;  
  
  for (i = 0; i < ADC_AVERAGING; i++)
  {
    ADS1x15_StartConversion(channelSetting);
    while (ADS1x15_ConversionReady() == false){} /* Wait for conversion ready */
    result += ADS1x15_GetRawResult();
  }
  
  return (result + ADC_AVERAGING / 2) / ADC_AVERAGING;
}

/**
 * Resets load to default state
 */
static void ResetLoad(void)
{
  // reset to CC, zero current, local voltage sensing
  AD569xR_Set(0);
  digitalWrite(CONTROL_CCCV_PIN, LOW);
  digitalWrite(AMMETER_GAIN_PIN, LOW);
  digitalWrite(VOLTMETER_GAIN_PIN, LOW);
  digitalWrite(VOLTMETER_4TERMINAL_PIN, LOW);
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
}


