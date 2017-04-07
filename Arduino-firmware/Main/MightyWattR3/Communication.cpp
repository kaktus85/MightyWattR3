/**
   Communication.cpp
   Communication from/to PC

   2016-10-24
   kaktus circuits
   GNU GPL v.3
*/


/* <Includes> */

#include "Arduino.h"
#include "Communication.h"
#include "Configuration.h"
#include "Ammeter.h"
#include "Voltmeter.h"
#include "CurrentSetter.h"
#include "VoltageSetter.h"
#include "Limiter.h"
#include "ErrorMessaging.h"
#include "LED.h"
#include "Fan.h"
#include "Control.h"
#include "Measurement.h"
#include "Thermometer.h"
#include "RangeSwitcher.h"
#include "Flashreader.h"
#include "MightyWatt.h"

/* </Includes> */


/* <Module variables> */

static const uint8_t dataLengthMapping[] = {0, 1, 2, COMMUNICATION_PAYLOAD_MAXIMUM_DATA_LENGTH};
static Communication_WriteCommand writeCommand; /* Present command from the PC */
static Communication_ReadCommand readCommand; /* Present command from the PC */
static uint8_t lastSent;
static ErrorMessaging_Error communicationError;
static uint8_t measurementMessage[COMMUNICATION_MEASUREMENT_MESSAGE_LENGTH];
static const Measurement_Values * measurementValues;
static const TSCUChar * temperature;
static char textMessage[64];

static const char Name[] FLASHMEMORY = NAME;
static const char CalibrationDate[] FLASHMEMORY = CALIBRATION_DATE;
static const char FirmwareVersion[] FLASHMEMORY = FIRMWARE_VERSION;
static const char BoardRevision[] FLASHMEMORY = BOARD_REVISION;

/* </Module variables> */


/* <Declarations (prototypes)> */

/**
   Receive part of the executable "Do" function handles incoming commands to set values to the load
*/
void Communication_Receive(void);

/**
   Send part of the executable "Do" function handles sending requested data
*/
void Communication_Send(void);

/* </Declarations (prototypes)> */


/* <Implementations> */

void Communication_Init(void)
{
  writeCommand.commandCounter = 0;
  readCommand.commandCounter = 0;
  lastSent = 0;

  Communication_Reset();

  communicationError.errorCounter = 0;
  communicationError.error = ErrorMessaging_Communication_CommandTimeout;
  measurementValues = Measurement_GetValues();
  temperature = Thermometer_GetTemperature();
}

void Communication_Do(void)
{
  if (SerialPort.available() > 0)  /* Receive message if data is available */
  {
    Communication_Receive();
  }
  Communication_Send();
}

void Communication_Reset(void)
{
  SerialPort.end();
  SerialPort.begin(COMMUNICATION_BAUDRATE);
  while(!SerialPort){}; /* Wait for the initialization of serial port */
  while(SerialPort.read() >= 0){}; /* Read all junk data already at the port */  
}

void Communication_Receive(void)
{
  int16_t data;
  uint32_t startTime;
  uint8_t i, dataLength; // dataLength is length of data payload without header and CRC
  uint16_t receivedCRC;
  static uint8_t message[COMMUNICATION_PAYLOAD_MAXIMUM_LENGTH + 1]; // payload + header

  data = SerialPort.read();
  if (data < 0)
  {
    return;
  }

  if (COMMUNICATION_COMMAND(data) == 0)
  {
     // null command is invalid
    return;
  }

  /* First byte is header */
  message[0] = (uint8_t)data;
  dataLength = dataLengthMapping[COMMUNICATION_DATA_LENGTH(message[0])];

  /* Read all incoming bytes */
  i = 0;
  startTime = millis();
  while (i < dataLength + COMMUNICATION_CRC_POLYNOMIAL_BYTE_LENGTH)
  {
    data = SerialPort.read();
    if (data >= 0)
    {
      message[i + 1] = (uint8_t)data;
      i++;
    }
    else if ((millis() - startTime) > COMMUNICATION_TIMEOUT)
    {
      /* timeout - error */
      communicationError.errorCounter++;
      communicationError.error = ErrorMessaging_Communication_CommandTimeout;
      return;
    }
  }

  /* Check CRC */
  receivedCRC = (uint16_t)message[dataLength + 1] | (((uint16_t)message[dataLength + 2]) << 8);
  if (receivedCRC != CRC16(COMMUNICATION_CRC_POLYNOMIAL_VALUE, message, 1 + dataLength)) // header + data
  {
    // CRC check fail - drop the message
    return;
  }

  /* Fill command structures */
  if (COMMUNICATION_RW(message[0]) == COMMUNICATION_WRITE)
  {
    /* Write to load */
    writeCommand.commandCounter++;
    writeCommand.command = COMMUNICATION_COMMAND(message[0]);
    for (i = 0; i < dataLength; i++) /* copy data */
    {
      writeCommand.data[i] = message[i + 1];
    }
    for (i; i < COMMUNICATION_PAYLOAD_MAXIMUM_DATA_LENGTH; i++) /* fill the rest with zeroes data */
    {
      writeCommand.data[i] = 0;
    }
  }
  else /* COMMUNICATION_READ */
  {
    /* Read from load (payload data discarded in this version) */
    readCommand.commandCounter++;
    readCommand.command = COMMUNICATION_COMMAND(message[0]);
  }
}

void Communication_Send(void)
{
  static uint8_t measurementValuesCounter = 0;    
  
  if (lastSent != readCommand.commandCounter)
  {
    uint8_t statusFlag = 0;
    uint32_t l;
    switch (readCommand.command)
    {
      case ReadCommand_IDN:
        Flashreader_Read((uint8_t*)textMessage, (uint8_t*)Name, sizeof(Name)/sizeof(Name[0]));
        SerialPort.println(textMessage);
        lastSent = readCommand.commandCounter;
        break;
      case ReadCommand_QDC:      
        Flashreader_Read((uint8_t*)textMessage, (uint8_t*)CalibrationDate, sizeof(CalibrationDate)/sizeof(CalibrationDate[0]));
        SerialPort.println(textMessage);
        Flashreader_Read((uint8_t*)textMessage, (uint8_t*)FirmwareVersion, sizeof(FirmwareVersion)/sizeof(FirmwareVersion[0]));
        SerialPort.println(textMessage);
        Flashreader_Read((uint8_t*)textMessage, (uint8_t*)BoardRevision, sizeof(BoardRevision)/sizeof(BoardRevision[0]));
        SerialPort.println(textMessage);
        SerialPort.println(CURRENT_SETTER_MAXIMUM_HICURRENT / 65536 * 65535);
        SerialPort.println(AMMETER_MAXIMUM_CURRENT / 65536 * 65535);
        SerialPort.println(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE / 65536 * 65535);
        SerialPort.println(VOLTMETER_MAXIMUM_VOLTAGE / 65536 * 65535);
        SerialPort.println(MAXIMUM_POWER);
        SerialPort.println(VOLTMETER_INPUT_RESISTANCE);
        SerialPort.println(LIMITER_MAXIMUM_TEMPERATURE);
        lastSent = readCommand.commandCounter;
        break;
      case ReadCommand_ErrorMessages:        
        uint8_t i;
        SerialPort.write(ErrorMessaging_ErrorNamesCount()); /* Send the message length in lines as the first byte */
        for (i = 0; i < ErrorMessaging_ErrorNamesCount(); i++)
        {
          ErrorMessaging_GetError(i, textMessage);
          SerialPort.println(textMessage);
        }
        lastSent = readCommand.commandCounter;
        break;
      case ReadCommand_Measurement:
        uint16_t crc;
        if (measurementValuesCounter != measurementValues->counter) /* Only send new measurement values */
        {
          l = measurementValues->current;
          measurementMessage[0] = l & 0xFF;
          measurementMessage[1] = (l >> 8) & 0xFF;
          measurementMessage[2] = (l >> 16) & 0xFF;
          measurementMessage[3] = (l >> 24) & 0xFF;

          l = measurementValues->voltage;
          measurementMessage[4] = l & 0xFF;
          measurementMessage[5] = (l >> 8) & 0xFF;
          measurementMessage[6] = (l >> 16) & 0xFF;
          measurementMessage[7] = (l >> 24) & 0xFF;

          measurementMessage[8] = temperature->value;

          if (Control_GetCCCV() == Control_CCCV_CV) /* Bit 0: Mode CV */
          {
            statusFlag |= 1 << 0;
          }
          if (RangeSwitcher_GetVoltageRange() == VoltageRange_LowVoltage) /* Bit 1: Low voltage range */
          {
            statusFlag |= 1 << 1;
          }
          if (RangeSwitcher_GetCurrentRange() == CurrentRange_LowCurrent) /* Bit 2: Low current range */
          {
            statusFlag |= 1 << 2;
          }
          if (LED_Get()) /* Bit 3: LED on */
          {
            statusFlag |= 1 << 3;
          }
          if (Fan_Get()) /* Bit 4: Fan on */
          {
            statusFlag |= 1 << 4;
          }
          if (Voltmeter_GetMode() == Voltmeter_4Terminal) /* Bit 5: 4-wire mode */
          {
            statusFlag |= 1 << 5;
          }

          measurementMessage[9] = statusFlag;
          l = ErrorMessaging_GetErrorFlags();

          measurementMessage[10] = l & 0xFF;
          measurementMessage[11] = (l >> 8) & 0xFF;
          measurementMessage[12] = (l >> 16) & 0xFF;
          measurementMessage[13] = (l >> 24) & 0xFF;

          // compute CRC of the measurement message body and append it to the end
          crc = CRC16(COMMUNICATION_CRC_POLYNOMIAL_VALUE, (const uint8_t *)measurementMessage, COMMUNICATION_MEASUREMENT_MESSAGE_DATA_LENGTH);
          measurementMessage[14] = crc & 0xFF;
          measurementMessage[15] = (crc >> 8) & 0xFF;

          SerialPort.write(measurementMessage, COMMUNICATION_MEASUREMENT_MESSAGE_LENGTH);
          measurementValuesCounter = measurementValues->counter;
          lastSent = readCommand.commandCounter;
        }
        break;
      default:
        lastSent = readCommand.commandCounter;
        break;
    }
  }
}

const Communication_WriteCommand * Communication_GetWriteCommand(void)
{
  return &writeCommand;
}

const Communication_ReadCommand * Communication_GetReadCommand(void)
{
  return &readCommand;
}

const ErrorMessaging_Error * Communication_GetError(void)
{
  return &communicationError;
}

uint16_t CRC16(const uint16_t polynomial, const uint8_t * data, uint8_t dataLength)
{
  uint16_t crc = 0;
  for (uint8_t i = 0; i < dataLength; i++)
  {
    crc ^= (((uint16_t)data[i]) << 8);
    for (uint8_t j = 0; j < 8; j++)
    {
      if ((crc & 0x8000U) > 0)
      {
        crc = (crc << 1) ^ polynomial;
      }
      else
      {
        crc = crc << 1;
      }
    }
  }
  return crc;
}

/* </Implementations> */
