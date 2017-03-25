/**
 * CommunicationWatchdog.cpp
 *
 * 2016-11-05
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "CommunicationWatchdog.h"
#include "Control.h"
#include "Communication.h"

/* </Includes> */ 


/* <Module variables> */ 

static const Communication_WriteCommand * writeCommand; /* Pointer to the write command where new write data from communication can be found */
static const Communication_ReadCommand * readCommand; /* Pointer to the write command where new read data from communication can be found */
static uint8_t writeCommandCounter, readCommandCounter; /* Number of the last received command from communication */
static uint32_t lastCommandMilliseconds; /* Time when the last command has been received */

/* </Module variables> */ 


/* <Implementations> */ 

void CommunicationWatchdog_Init(void)
{  
  writeCommand = Communication_GetWriteCommand();
  readCommand = Communication_GetReadCommand();
  writeCommandCounter = writeCommand->commandCounter;
  readCommandCounter = readCommand->commandCounter;
  lastCommandMilliseconds = millis();
}

void CommunicationWatchdog_Do(void)
{
  if (writeCommandCounter != writeCommand->commandCounter)
  {
    lastCommandMilliseconds = millis();
    writeCommandCounter = writeCommand->commandCounter;
  }
 
  if (readCommandCounter != readCommand->commandCounter)
  {
    lastCommandMilliseconds = millis();
    readCommandCounter = readCommand->commandCounter;
  }
  
  if (millis() - lastCommandMilliseconds > COMMUNICATION_WATCHDOG_TIMEOUT)
  {
    /* Communication timeout */
    lastCommandMilliseconds = millis();
    Control_StopLoad();
    Communication_Reset(); /* Reset COM port */
    MightyWatt_Init();
  }
}
  
/* </Implementations> */ 
