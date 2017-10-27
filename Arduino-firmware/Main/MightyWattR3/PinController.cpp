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


/* <Defines and macros> */

#define PINCONTROLLER_ISSET(x)    ((((x) >> 7) & 0x1) > 0)

/* </Defines and macros> */ 


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
        if (PINCONTROLLER_ISSET((writeCommand->data)[0]))
        {
          // set pins
          Pin_Set(PinController_GetPins() | (writeCommand->data)[0] & 0x7F);
        }
        else
        {
          // reset pins
          Pin_Set(PinController_GetPins() & ~((writeCommand->data)[0]) & 0x7F);
        }
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

