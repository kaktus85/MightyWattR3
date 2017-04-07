/**
 * Communication.h
 *
 * 2016-10-24
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

 /* <Includes> */ 
 
#include "MightyWatt.h"
#include "ErrorMessaging.h"

 /* </Includes> */ 
 

/* <Defines> */ 

#define COMMUNICATION_PAYLOAD_MAXIMUM_DATA_LENGTH       4
#define COMMUNICATION_PAYLOAD_MAXIMUM_LENGTH            (COMMUNICATION_PAYLOAD_MAXIMUM_DATA_LENGTH + COMMUNICATION_CRC_POLYNOMIAL_BYTE_LENGTH)
#define COMMUNICATION_BAUDRATE                          500000
#define COMMUNICATION_TIMEOUT                           200 /* ms */
#define COMMUNICATION_RW(x)                             ((x & 0x80) >> 7)
#define COMMUNICATION_DATA_LENGTH(x)                    ((x & 0x60) >> 5) /* 0 = 0 bytes, 1 = 1 byte, 2 = 2 bytes, 3 = 4 bytes*/
#define COMMUNICATION_COMMAND(x)                        (x & 0x1F)
#define COMMUNICATION_CRC_POLYNOMIAL_BYTE_LENGTH        2
#define COMMUNICATION_CRC_POLYNOMIAL_VALUE              0x1021U /* CRC-16 CCITT */
#define COMMUNICATION_MEASUREMENT_MESSAGE_DATA_LENGTH   14
#define COMMUNICATION_MEASUREMENT_MESSAGE_LENGTH        (COMMUNICATION_MEASUREMENT_MESSAGE_DATA_LENGTH + COMMUNICATION_CRC_POLYNOMIAL_BYTE_LENGTH)
#define COMMUNICATION_READ                              0
#define COMMUNICATION_WRITE                             1

/* </Defines> */ 


/* <Enums> */ 

/**
 * Write commands
 * Max 31
 */
enum Communication_WriteCommands : uint8_t
{
  WriteCommand_Invalid = 0, /* command will be ignored */
  WriteCommand_ConstantCurrent = 1,
  WriteCommand_ConstantVoltage = 2,
  WriteCommand_ConstantPowerCC = 3,
  WriteCommand_ConstantPowerCV = 4,
  WriteCommand_ConstantResistanceCC = 5,
  WriteCommand_ConstantResistanceCV = 6,
  WriteCommand_ConstantVoltageSoftware = 7,
  WriteCommand_MPPT = 8,
  WriteCommand_SeriesResistance = 10,
  WriteCommand_4Wire = 11,
  WriteCommand_MeasurementSpeed = 12,
  WriteCommand_FanRules = 13,
  WriteCommand_LEDRules = 14,
  WriteCommand_LEDBrightness = 15,
  WriteCommand_CurrentRangeAuto = 16,
  WriteCommand_VoltageRangeAuto = 17,
};

/**
 * Read commands
 * Max 31
 */
enum Communication_ReadCommands : uint8_t
{
  ReadCommand_Invalid = 0, /* command will be ignored */
  ReadCommand_Measurement = 1,
  ReadCommand_IDN = 2,
  ReadCommand_QDC = 3,
  ReadCommand_ErrorMessages = 4
};

/* </Enums> */ 


/* <Structs> */ 

/**
 * Structure for handling information that are to be handled by the load (e. g. set current)
 */
struct Communication_WriteCommand
{
  uint8_t commandCounter; /* "Unique" number of the received command for identification. Intentional wraparound. Useful for identification if the command has been processed. */
  uint8_t command; /* Number indicating what the load is supposed to do */
  uint8_t data[COMMUNICATION_PAYLOAD_MAXIMUM_DATA_LENGTH]; /* Generic data for the command - will be interpreted based on command number */
};

/**
 * Structure for handling outgoing data that are to be sent by the load (e. g. read temperature)
 */
struct Communication_ReadCommand
{
  uint8_t commandCounter; /* "Unique" number of the received command for identification. Intentional wraparound. Useful for identification if the command has been processed. */
  uint8_t command; /* Number indicating what the load is supposed to do */
};

/* </Structs> */ 


/* <Declarations (prototypes)> */ 

/**
 * Initializes serial port
 */
void Communication_Init(void);

/**
 * Executable function which must be called periodically
 */
void Communication_Do(void);

/**
 * Closes and reopens serial port
 */
void Communication_Reset(void);

/**
 * Gets the present write command
 *
 * @return - Pointer to the write command
 */
const Communication_WriteCommand * Communication_GetWriteCommand(void);

/**
 * Gets the present read command
 *
 * @return - Pointer to the read command
 */
const Communication_ReadCommand * Communication_GetReadCommand(void);

/**
 * Returns error structure for this module
 *
 * @return - Pointer to constant error structure
 */
const ErrorMessaging_Error * Communication_GetError(void);

/**
 * Computes 16-bit cyclic redundancy check on array of binary data
 *
 * @param polynomial - CRC polynomial
 * @param data - pointer to array of binary data for which the CRC will be calculated
 * @param dataLength - length of data
 *
 * @return - 16-bit CRC of the data
 */
uint16_t CRC16(const uint16_t polynomial, const uint8_t * data, uint8_t dataLength);

/* </Declarations (prototypes)> */ 

#endif /* COMMUNICATION_H */
