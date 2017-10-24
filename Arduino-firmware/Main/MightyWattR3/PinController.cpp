/**
 * PinController.cpp
 *
 * 2017-10-24
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "Pin.h"
#include "Communication.h"
#include "PinController.h"

/* </Includes> */ 


/* <Module variables> */ 

static const Communication_WriteCommand * writeCommand; /* Pointer to the write command where new data from communication can be found */
static uint8_t commandCounter; /* Number of the last executed command from communication */

/* </Module variables> */ 


/* <Implementations> */ 

void PinController_Init(void)
{
  writeCommand = Communication_GetWriteCommand();  
  commandCounter = 0;
}

void PinController_Do(void)
{
  /* Check new command */
  if (writeCommand->commandCounter != commandCounter)
  {
    /* LSB first */
    switch (writeCommand->command)
    {
      case WriteCommand_Pins:
        Pin_Set((writeCommand->data)[0]);
      break;
      default:
      /* command handled by other modules */
      break;
    }
    commandCounter = writeCommand->commandCounter;
  }  
}

uint8_t PinController_GetPins(void)
{
  return Pin_Get();
}

/* </Implementations> */ 

