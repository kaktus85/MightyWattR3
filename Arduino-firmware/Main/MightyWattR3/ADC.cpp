/**
 * ADC.cpp
 * ADC management
 *
 * 2016-10-21
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "ADC.h"

/* </Includes> */ 


/* <Module variables> */ 

static ADS1x15_ChannelSetting ChannelSettings[ADC_CHANNEL_COUNT];
static bool ChannelIsFiltered[ADC_CHANNEL_COUNT];
static uint8_t ChannelSkipRatio[ADC_CHANNEL_COUNT];
static uint16_t ChannelCycleCounter[ADC_CHANNEL_COUNT];
static uint32_t LastUpdate;
static TSCADCLong Voltages[ADC_CHANNEL_COUNT];
static ErrorMessaging_Error ADCError[ADC_CHANNEL_COUNT];
static int32_t VoltageFilterData[ADC_V_CHANNEL_FILTER_SIZE],
               CurrentFilterData[ADC_I_CHANNEL_FILTER_SIZE],
               TemperatureFilterData[ADC_T_CHANNEL_FILTER_SIZE];
static ADC_TriangleFilterData VoltageFilter = {ADC_V_CHANNEL_FILTER_SIZE, VoltageFilterData, 0, 0, 0, false };
static ADC_TriangleFilterData CurrentFilter = {ADC_I_CHANNEL_FILTER_SIZE, CurrentFilterData, 0, 0, 0, false };
static ADC_TriangleFilterData TemperatureFilter = {ADC_T_CHANNEL_FILTER_SIZE, TemperatureFilterData, 0, 0, 0, false };
static ADC_TriangleFilterData Filters[ADC_CHANNEL_COUNT] = {VoltageFilter, CurrentFilter, TemperatureFilter};

/* </Module variables> */ 


/* <Declarations (prototypes)> */ 

/**
 * Adds a value to the triangle filter
 * 
 * @param value - new value to add
 * @param filter - pointer to triangle filter data
 */
void TriangleFilter_Add(int32_t value, ADC_TriangleFilterData * filter);

/**
 * Gets filtered value with triangle-weighted filter
 * 
 * @param filter - pointer to triangle filter data
 *
 * @return - filtered value if filter is valid, last value otherwise
 */
int32_t TriangleFilter_GetValue(ADC_TriangleFilterData * filter);

/**
 * Gets the last added value to the filter
 * 
 * @param filter - pointer to triangle filter data
 *
 * @return - last value (unfiltered)
 */
int32_t TriangleFilter_GetUnfilteredValue(ADC_TriangleFilterData * filter);

/* </Declarations (prototypes)> */ 


/* <Implementations> */ 

void ADC_Init(void)
{
  ChannelSettings[ADC_V].input = ADC_V_CHANNEL;
  ChannelSettings[ADC_I].input = ADC_I_CHANNEL;
  ChannelSettings[ADC_T].input = ADC_T_CHANNEL;
  
  ChannelSettings[ADC_V].range = ADC_DEFAULT_RANGE;
  ChannelSettings[ADC_I].range = ADC_DEFAULT_RANGE;
  ChannelSettings[ADC_T].range = ADC_DEFAULT_RANGE;
  
  #ifdef ADC_TYPE_ADS1015
    ChannelSettings[ADC_V].dataRate = ADS1015_920SPS;
    ChannelSettings[ADC_I].dataRate = ADS1015_920SPS;
    ChannelSettings[ADC_T].dataRate = ADS1015_920SPS;
  #elif defined(ADC_TYPE_ADS1115)
    ChannelSettings[ADC_V].dataRate = ADS1115_860SPS;
    ChannelSettings[ADC_I].dataRate = ADS1115_860SPS;
    ChannelSettings[ADC_T].dataRate = ADS1115_860SPS;
  #else
    #error No ADC defined
  #endif
  
  ChannelSettings[ADC_V].autorange = true;
  ChannelSettings[ADC_I].autorange = true;
  ChannelSettings[ADC_T].autorange = false;
  
  ChannelIsFiltered[ADC_V] = true;
  ChannelIsFiltered[ADC_I] = true;
  ChannelIsFiltered[ADC_T] = false;

  ChannelSkipRatio[ADC_V] = ADC_V_CHANNEL_SKIP_RATIO;
  ChannelSkipRatio[ADC_I] = ADC_I_CHANNEL_SKIP_RATIO;
  ChannelSkipRatio[ADC_T] = ADC_T_CHANNEL_SKIP_RATIO;

  ChannelCycleCounter[ADC_V] = 0;
  ChannelCycleCounter[ADC_I] = 0;
  ChannelCycleCounter[ADC_T] = 0;
  
  int16_t i;
  for (i = 0; i < ADC_CHANNEL_COUNT; i++)
  {
    Voltages[i].milliseconds = 0;
    Voltages[i].counter = 0;
    Voltages[i].value = 0;
    Voltages[i].unfilteredValue = 0;
    ADCError[i].errorCounter = 0;
    ADCError[i].error = ErrorMessaging_ADC_Overload;
  }
  
  ADS1x15_Init();
  ADS1x15_StartConversion(ChannelSettings[0]); /* Start conversion of the first channel */
  LastUpdate = millis();  
}

void ADC_Do(void) /* Call periodically */
{
  static uint8_t i = 0; /* Channel iterator */
  static bool repeatedConversion = false;
  static int16_t rawResult;  
  
  if (ADS1x15_ConversionReady())
  {
    repeatedConversion = false;
    rawResult = ADS1x15_GetRawResult();    
    int32_t result = ADS1x15_Voltage(rawResult, ChannelSettings[i].range); /* Get the new voltage */         
    
    if ((result > ADC_ABSOLUTEMAXIMUM) || (result < -ADC_ABSOLUTEMAXIMUM))
    {
      /* ADC negative or positive overload */
      ADCError[i].errorCounter++;
      ADCError[i].error = ErrorMessaging_ADC_Overload;      
    }
    TriangleFilter_Add(result, &Filters[i]);
    Voltages[i].unfilteredValue = TriangleFilter_GetUnfilteredValue(&Filters[i]);
    if (ChannelIsFiltered[i])
    {
      Voltages[i].value = TriangleFilter_GetValue(&Filters[i]);
    }
    else
    {
      Voltages[i].value = Voltages[i].unfilteredValue;
    }
    
    Voltages[i].milliseconds = millis();
    Voltages[i].counter++;    

    if (ChannelSettings[i].autorange) /* Autoranging, if enabled */
    {
      ADS1x15_AutoRange(rawResult, &(ChannelSettings[i].range));
    }
    else /* Default range, if autoranging is disabled */
    {
      ChannelSettings[i].range = ADC_DEFAULT_RANGE;
    }
    LastUpdate = millis();

    do
    {      
      for (uint8_t j = 0; j < ADC_CHANNEL_COUNT; j++)
      {
        i++;
        if (i == ADC_CHANNEL_COUNT) /* Wrap around the number of channels */
        {
          i = 0;
        }
        ChannelCycleCounter[i]++; /* Increase cycle number */

        if ((ChannelCycleCounter[i] & ((1U << ChannelSkipRatio[i]) - 1U)) == 0) /* Try finding the next channel that is not skipped */
        {
          break;
        }
      }
    } while ((ChannelCycleCounter[i] & ((1U << ChannelSkipRatio[i])) - 1U) > 0); /* Channel skipping */
    
    ADS1x15_StartConversion(ChannelSettings[i]); /* Start converting the next channel */
  }
  
  if ((millis() - LastUpdate) > ADC_TIMEOUT)
  {
    if (repeatedConversion == false)
    {
      repeatedConversion = true;
      LastUpdate = millis();
      ADS1x15_StartConversion(ChannelSettings[i]); /* Try repeating the last conversion */  
    }
    else
    {
      /* Error: ADC not responding */
      ADCError[i].errorCounter++;
      ADCError[i].error = ErrorMessaging_ADC_NotResponding;
      repeatedConversion = false;
    }
  }
}

void ADC_SetupChannel(ADC_Channels adcChannel, ADC_RateRangingFilter rateRangingFilter)
{
  ChannelSettings[adcChannel].dataRate = rateRangingFilter.dataRate;
  ChannelSettings[adcChannel].autorange = rateRangingFilter.autorange;
  ChannelIsFiltered[adcChannel] = rateRangingFilter.filter;
}

const TSCADCLong * ADC_GetVoltage(ADC_Channels adcChannel)
{
  return &(Voltages[adcChannel]);
}

const ErrorMessaging_Error * ADC_GetError(ADC_Channels adcChannel)
{
  return &(ADCError[adcChannel]);
}

void TriangleFilter_Add(int32_t value, ADC_TriangleFilterData * filter)
{
  filter->triangleSum -= filter->sum;
  filter->sum -= (filter->data)[filter->index];
  (filter->data)[filter->index] = value;
  filter->triangleSum += value * (int32_t)(filter->filterSize);
  filter->sum += value;
  
  filter->index++;
  if (filter->index >= filter->filterSize)
  {
    filter->index = 0;
    filter->valid = true;
  }
}

int32_t TriangleFilter_GetValue(ADC_TriangleFilterData * filter)
{
  if (filter->valid)
  {  
    int32_t totalWeight = ((int32_t)(filter->filterSize) * ((int32_t)(filter->filterSize) + 1)) / 2;
    return ((int32_t)(filter->triangleSum) + totalWeight / 2) / totalWeight;
  }
  else
  {
    return TriangleFilter_GetUnfilteredValue(filter);
  }
}

int32_t TriangleFilter_GetUnfilteredValue(ADC_TriangleFilterData * filter)
{
  if (filter->index > 0)
  {
    return (filter->data)[filter->index - 1];
  }
  else
  {
    return (filter->data)[filter->filterSize - 1];
  }
}

void ADC_ResetFilter(ADC_Channels adcChannel)
{
  if (adcChannel < ADC_CHANNEL_COUNT)
  {
      ADC_TriangleFilterData * filter = &(Filters[adcChannel]);
      int32_t lastValue = TriangleFilter_GetUnfilteredValue(filter);
      
      // Reset filter
      filter->index = 0;
      filter->sum = 0;
      filter->triangleSum = 0;
      filter->valid = false;
      memset(filter->data, 0, filter->filterSize * sizeof(int32_t));

      TriangleFilter_Add(lastValue, filter); // add the last value
   }
}

/* </Implementations> */ 
