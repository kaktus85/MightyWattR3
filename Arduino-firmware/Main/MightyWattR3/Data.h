/**
 * Data.h
 *
 * 2016-10-28
 * kaktus circuits
 * GNU GPL v.3
 */
 
#ifndef DATA_H
#define DATA_H

/* <Includes> */ 

#include "MightyWatt.h"

/* </Includes> */ 

 
/* <Structs> */ 

/**
 * (T)ime(s)tamped (c)ounted (u)nchar
 * Timestamp should contain millisecond counter
 * Counter should update every time new value is written
 */
struct TSCUChar
{
  uint32_t milliseconds;
  uint8_t counter;
  uint8_t value;
};

/**
 * (T)ime(s)tamped (c)ounted int8_t
 * Timestamp should contain millisecond counter
 * Counter should update every time new value is written
 */
struct TSCChar
{
  uint32_t milliseconds;
  uint8_t counter;
  int8_t value;
};

/**
 * (T)ime(s)tamped (c)ounted (u)ninteger
 * Timestamp should contain millisecond counter
 * Counter should update every time new value is written
 */
struct TSCUInt
{
  uint32_t milliseconds;
  uint8_t counter;
  uint16_t value;
};

/**
 * (T)ime(s)tamped (c)ounted integer
 * Timestamp should contain millisecond counter
 * Counter should update every time new value is written
 */
struct TSCInt
{
  uint32_t milliseconds;
  uint8_t counter;
  int16_t value;
};

/**
 * (T)ime(s)tamped (c)ounted (u)nint32_t
 * Timestamp should contain millisecond counter
 * Counter should update every time new value is written
 */
struct TSCULong
{
  uint32_t milliseconds;
  uint8_t counter;
  uint32_t value;
};

/**
 * (T)ime(s)tamped (c)ounted int32_t
 * Timestamp should contain millisecond counter
 * Counter should update every time new value is written
 */
struct TSCLong
{
  uint32_t milliseconds;
  uint8_t counter;
  int32_t value;
};

/**
 * (T)ime(s)tamped (c)ounted ADC (u)int32_t
 * Timestamp should contain millisecond counter
 * Counter should update every time new value is written
 */
struct TSCADCULong
{
  uint32_t milliseconds;
  uint8_t counter;
  uint32_t value;
  uint32_t unfilteredValue;
};

/**
 * (T)ime(s)tamped (c)ounted ADC int32_t
 * Timestamp should contain millisecond counter
 * Counter should update every time new value is written
 */
struct TSCADCLong
{
  uint32_t milliseconds;
  uint8_t counter;
  int32_t value;
  int32_t unfilteredValue;
};

/* </Structs> */ 


/* <Declarations (prototypes)> */ 

/**
 * Return an uint32_t number from array of unchars
 *
 * @param value[] - array of uint8_t from which to construct the result, LSB first
 *
 * @return - Value as an uint32_t
 */
inline uint32_t Data_GetULongFromUCharArray(const uint8_t value[])
{
  return ((uint32_t)(value[3])) << 24 |
         ((uint32_t)(value[2])) << 16 |
         ((uint32_t)(value[1])) << 8 |
          (uint32_t)(value[0]);
}

/**
 * Return an uint16_t number from array of unchars
 *
 * @param value[] - array of uint8_t from which to construct the result, LSB first
 *
 * @return - Value as an uint16_t
 */
inline uint16_t Data_GetUIntFromUCharArray(const uint8_t value[])
{
  return ((uint16_t)(value[1])) << 8 |
          (uint16_t)(value[0]);
}

/* </Declarations (prototypes)> */ 

#endif /* DATA_H */
