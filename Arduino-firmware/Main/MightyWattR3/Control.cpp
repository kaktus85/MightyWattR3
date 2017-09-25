/**
 * Control.cpp
 *
 * 2016-10-25
 * kaktus circuits
 * GNU GPL v.3
 */


/* <Includes> */ 

#include "Arduino.h"
#include "Ammeter.h"
#include "Voltmeter.h"
#include "DACC.h"
#include "Communication.h"
#include "Configuration.h"
#include "Control.h"
#include "Data.h"
#include "CurrentSetter.h"
#include "VoltageSetter.h"
#include "RangeSwitcher.h"

/* </Includes> */ 


/* <Module variables> */ 

static uint32_t setCurrent, setVoltage, setPower, setResistance; /* Values to be set */
//static RangeSwitcher_CurrentRanges ammeterRangeWhenSet; /* Stores the ammeter range when voltage was set to DAC */
//static Voltmeter_Ranges voltmeterRangeWhenSet; /* Stores the voltmeter range when voltage was set to DAC */
void (* Control_Keep)(void); /* Pointer to the constant keeper function */
static const Communication_WriteCommand * writeCommand; /* Pointer to the write command where new data from communication can be found */
static uint8_t commandCounter = 0; /* Number of the last executed command from communication */
static const Measurement_Values * measurementValues; /* Pointer to the latest measured voltage, current, power and resistance */
static uint8_t measurementCounter; /* Number of the last processed measurement data */
static uint32_t measurementTimer; /* Time of the last processed measurement data */
static uint32_t lastCurrent, lastPower, lastResistance, lastVoltage, lastLastPower; /* Saved values for software modes */
static ErrorMessaging_Error ControlError;
const static ErrorMessaging_Error * CurrentSetterError;
const static ErrorMessaging_Error * VoltageSetterError;
static uint8_t currentSetterErrorCounter, voltageSetterErrorCounter;
static Control_CCCVStates cccvState;
static uint32_t stepSize; /* Software control loop step size */
static bool MPPT_initialized;

/* </Module variables> */ 


/* <Declarations (prototypes)> */ 

/**
 * Sets the present value of "setCurrent" to DAC
 */
void Control_SetCurrent(void);

/**
 * Keeps constant current
 */
void Control_KeepCurrent(void);

/**
 * Sets the present value of "setVoltage" to DAC
 */
void Control_SetVoltage(void);

/**
 * Keeps constant voltage
 */
void Control_KeepVoltage(void);

/**
 * Set-ups constant power logic using constant current mode
 */
void Control_SetPowerCC(void);

/**
 * Keeps constant power using constant current mode
 */
void Control_KeepPowerCC(void);

/**
 * Set-ups constant power logic using constant voltage mode
 */
void Control_SetPowerCV(void);

/**
 * Keeps constant power using constant voltage mode
 */
void Control_KeepPowerCV(void);

/**
 * Set-ups constant resistance logic using constant current mode
 */
void Control_SetResistanceCC(void);

/**
 * Keeps constant resistance using constant current mode
 */
void Control_KeepResistanceCC(void);

/**
 * Set-ups constant resistance logic using constant voltage mode
 */
void Control_SetResistanceCV(void);

/**
 * Keeps constant resistance using constant voltage mode
 */
void Control_KeepResistanceCV(void);

/**
 * Set-ups constant voltage software control loop logic
 */
void Control_SetVoltageSoftware(void);

/**
 * Keeps constant voltage in software control loop (physically CC)
 */
void Control_KeepVoltageSoftware(void);

/**
 * Set-ups control logic for maximum power point tracker
 * Starting current is obtained from communication command
 */
void Control_SetMPPT(void);

/**
 * Keeps maximum power point using a software control loop (physically CC)
 */
void Control_KeepMPPT(void);

/*
 * Sets the maximum current at the present range
 * Used for simple ammeter
 */
void Control_SetMaxCurrent(void);

/**
 * Increases step size for CC software-controlled modes
 * Checks and maintains maximum step size
 */
void Control_StepSizeCurrentPlus(void);

/**
 * Decreases step size for CC software-controlled modes
 * Checks and maintains minimum step size
 */
void Control_StepSizeCurrentMinus(void);

/**
 * Provides limitation of the step size
 * 
 * @param stepSize - pointer to the step size that will be altered to fit into the limits
 */
void Control_LimitCurrentStepSize(uint32_t * stepSize);

/**
 * Increases step size for CV software-controlled modes
 * Checks and maintains maximum step size
 */
void Control_StepSizeVoltagePlus(void);

/**
 * Decreases step size for CV software-controlled modes
 * Checks and maintains minimum step size
 */
void Control_StepSizeVoltageMinus(void);

/**
 * Provides limitation of the step size
 * 
 * @param stepSize - pointer to the step size that will be altered to fit into the limits
 */
void Control_LimitVoltageStepSize(uint32_t * stepSize);

/**
 * Generic software control loop for CC mode
 * 
 * @param setValue - the target value to reach
 * @param lastValue - pointer to measured value before the last action
 * @param presentValue - last measured value
 * @param lastAction - pointer to last action (current up or down)
 */
void Control_SWCC(uint32_t setValue, uint32_t * lastValue, uint32_t presentValue, Control_CurrentActions * lastAction);

/**
 * Generic software control loop for CV mode
 * 
 * @param setValue - the target value to reach
 * @param lastValue - pointer to measured value before the last action
 * @param presentValue - last measured value
 * @param lastAction - pointer to last action (current up or down)
 */
void Control_SWCV(uint32_t setValue, uint32_t * lastValue, uint32_t presentValue, Control_VoltageActions * lastAction);

/* </Declarations (prototypes)> */ 


/* <Implementations> */ 

void Control_Init(void)
{
  pinMode(CONTROL_CCCV_PIN, OUTPUT);
  Control_StopLoad();
  writeCommand = Communication_GetWriteCommand();
  measurementValues = Measurement_GetValues();
  measurementCounter = 0;
  ControlError.errorCounter = 0;
  ControlError.error = CurrentSetterError->error;
  CurrentSetterError = CurrentSetter_GetError();
  VoltageSetterError = VoltageSetter_GetError();  
}

void Control_Do(void)
{
  /* Check new command */
  if (writeCommand->commandCounter != commandCounter)
  {
    /* LSB first */
    switch (writeCommand->command)
    {
      case WriteCommand_ConstantCurrent:
        setCurrent = Data_GetULongFromUCharArray(writeCommand->data);
        Control_SetCurrent();
        Control_Keep = &Control_KeepCurrent;
      break;
      case WriteCommand_ConstantVoltage:
        setVoltage = Data_GetULongFromUCharArray(writeCommand->data);
        Control_SetVoltage();
        Control_Keep = &Control_KeepVoltage;
      break;
      case WriteCommand_ConstantPowerCC:
        setPower = Data_GetULongFromUCharArray(writeCommand->data);
        Control_SetPowerCC();
        Control_Keep = &Control_KeepPowerCC;
      break;
      case WriteCommand_ConstantPowerCV:
        setPower = Data_GetULongFromUCharArray(writeCommand->data);
        Control_SetPowerCV();
        Control_Keep = &Control_KeepPowerCV;
      break;
      case WriteCommand_ConstantResistanceCC:
        setResistance = Data_GetULongFromUCharArray(writeCommand->data);
        Control_SetResistanceCC();
        Control_Keep = &Control_KeepResistanceCC;
      break;
      case WriteCommand_ConstantResistanceCV:
        setResistance = Data_GetULongFromUCharArray(writeCommand->data);
        Control_SetResistanceCV();
        Control_Keep = &Control_KeepResistanceCV;
      break;
      case WriteCommand_ConstantVoltageSoftware:
        setVoltage = Data_GetULongFromUCharArray(writeCommand->data);
        Control_SetVoltageSoftware();
        Control_Keep = &Control_KeepVoltageSoftware;
      break;
      case WriteCommand_MPPT:
        //setCurrent = Data_GetULongFromUCharArray(writeCommand->data);            
        setVoltage = Data_GetULongFromUCharArray(writeCommand->data);     
        Control_SetMPPT();
        Control_Keep = &Control_KeepMPPT;     
      break;
      case WriteCommand_SimpleAmmeter:
        Control_SetMaxCurrent();
        Control_Keep = NULL; // No keeper necessary
      break;      
      default:
      /* command handled by other modules */
      break;
    }
    commandCounter = writeCommand->commandCounter;
  }  
  
  if (Control_Keep != NULL)
  {
    Control_Keep();
  }

  if (currentSetterErrorCounter != CurrentSetterError->errorCounter)
  {
    currentSetterErrorCounter = CurrentSetterError->errorCounter;
    ControlError.errorCounter++;
    ControlError.error = CurrentSetterError->error;
  }

  if (voltageSetterErrorCounter != VoltageSetterError->errorCounter)
  {
    voltageSetterErrorCounter = VoltageSetterError->errorCounter;
    ControlError.errorCounter++;
    ControlError.error = VoltageSetterError->error;
  }
}

void Control_StopLoad(void)
{
  setCurrent = 0;
  CurrentSetter_SetZero();
  Control_Keep = &Control_KeepCurrent;
}

void Control_SetCurrent(void)
{  
  CurrentSetter_SetCurrent(setCurrent);
}

void Control_KeepCurrent(void)
{
  CurrentSetter_Do();
}

void Control_SetVoltage(void)
{  
  VoltageSetter_SetVoltage(setVoltage);
}

void Control_KeepVoltage(void)
{
  VoltageSetter_Do();
}

void Control_SetPowerCC(void)
{
  stepSize = 0;
  Control_LimitCurrentStepSize(&stepSize);
  lastPower = measurementValues->unfilteredPower;
  
  if ((setPower > 0) && (measurementValues->unfilteredVoltage > VOLTMETER_THRESHOLD_VOLTAGE)) // initial estimate I = P/V
  {     
    uint64_t current = (((uint64_t)setPower) * 1000000) / measurementValues->unfilteredVoltage;

    if (current >= (uint64_t)CURRENT_SETTER_MAXIMUM_HICURRENT) 
    {
      CurrentSetter_SetCurrent((uint32_t)(CURRENT_SETTER_MAXIMUM_HICURRENT - 1));
    }
    else
    {
      CurrentSetter_SetCurrent((uint32_t)current);
    }
  }
  else
  {
    CurrentSetter_SetCurrent(0);
  }
}

void Control_KeepPowerCC(void)
{
  static Control_CurrentActions lastAction = Control_CurrentUp;
  
  if (measurementValues->milliseconds - measurementTimer > CONTROL_BANDWIDTH_LIMIT_CC)
  {
    if ((setPower > 0) && (measurementValues->unfilteredVoltage > VOLTMETER_THRESHOLD_VOLTAGE))
    { 
      Control_SWCC(setPower, &lastPower, measurementValues->unfilteredPower, &lastAction);
    }
    else
    {
      CurrentSetter_SetCurrent(0);
      stepSize = 0;
      Control_LimitCurrentStepSize(&stepSize);
    }
    measurementTimer = measurementValues->milliseconds;
  }
  CurrentSetter_Do();
}

void Control_SetPowerCV(void)
{
  stepSize = 0;
  Control_LimitVoltageStepSize(&stepSize);
  lastPower = measurementValues->unfilteredPower;

  if ((setPower > 0))
  { 
    uint64_t voltage;
    
    if (measurementValues->unfilteredCurrent > AMMETER_THRESHOLD_VOLTAGE)
    {
      voltage = (((uint64_t)setPower) * 1000000) / measurementValues->unfilteredCurrent; // initial estimate V = P/I
    }
    else
    {
      voltage = measurementValues->unfilteredVoltage; // initial estimate V = measured V
    }

    if (voltage >= (uint64_t)VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE)
    {      
      VoltageSetter_SetVoltage((uint32_t)(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE - 1)); 
    }
    else
    {      
      VoltageSetter_SetVoltage((uint32_t)voltage); 
    }
  }
  else
  {
    VoltageSetter_SetVoltage((uint32_t)(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE - 1));
  }
}

void Control_KeepPowerCV(void)
{
  static Control_VoltageActions lastAction = Control_VoltageDown;
  
  if (measurementValues->milliseconds - measurementTimer > CONTROL_BANDWIDTH_LIMIT_CV)
  {
    if ((setPower > 0) && (measurementValues->unfilteredVoltage > VOLTMETER_THRESHOLD_VOLTAGE))
    { 
      Control_SWCV(setPower, &lastPower, measurementValues->unfilteredPower, &lastAction);
    }
    else
    {
      VoltageSetter_SetVoltage((uint32_t)(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE - 1));      
      stepSize = 0;
      Control_LimitVoltageStepSize(&stepSize);
    }
    measurementTimer = measurementValues->milliseconds;
  }
  VoltageSetter_Do();
}

void Control_SetResistanceCC(void)
{
  stepSize = 0;
  Control_LimitCurrentStepSize(&stepSize);
  lastResistance = measurementValues->unfilteredResistance;
  if ((setResistance < VOLTMETER_INPUT_RESISTANCE) && (measurementValues->unfilteredVoltage > VOLTMETER_THRESHOLD_VOLTAGE)) // initial estimate I = V/R
  { 
    if (setResistance >= VOLTMETER_INPUT_RESISTANCE)
    {    
      CurrentSetter_SetCurrent(0); 
    }
    else if (setResistance > 0)
    {
      uint64_t current = (((uint64_t)measurementValues->unfilteredVoltage) * 1000) / setResistance;

      if (current >= (uint64_t)CURRENT_SETTER_MAXIMUM_HICURRENT) 
      {
        CurrentSetter_SetCurrent((uint32_t)(CURRENT_SETTER_MAXIMUM_HICURRENT - 1));
      }
      else
      {
        CurrentSetter_SetCurrent((uint32_t)current);
      }
    }  
    else
    {
      // Set maximum current on zero resistance
      CurrentSetter_SetCurrent((uint32_t)(CURRENT_SETTER_MAXIMUM_HICURRENT - 1));
    }
  }
}

void Control_KeepResistanceCC(void)
{
  static Control_CurrentActions lastAction = Control_CurrentUp;
  
  if (measurementValues->milliseconds - measurementTimer > CONTROL_BANDWIDTH_LIMIT_CC)
  {    
    if (setResistance >= VOLTMETER_INPUT_RESISTANCE)
    {
      CurrentSetter_SetCurrent(0);
    }
    else if (setResistance > 0)
    {            
      Control_SWCC(setResistance, &lastResistance, measurementValues->unfilteredResistance, &lastAction);
    }        
    else
    {
      CurrentSetter_SetCurrent((uint32_t)(CURRENT_SETTER_MAXIMUM_HICURRENT - 1));
    } 
    measurementTimer = measurementValues->milliseconds;    
  }  
  CurrentSetter_Do();
}

void Control_SetResistanceCV(void)
{
  stepSize = 0;
  Control_LimitVoltageStepSize(&stepSize);
  lastResistance = measurementValues->unfilteredResistance;
  if ((setResistance < VOLTMETER_INPUT_RESISTANCE) && (measurementValues->unfilteredCurrent > AMMETER_THRESHOLD_VOLTAGE)) // initial estimate V = R * I
  { 
    if (setResistance >= VOLTMETER_INPUT_RESISTANCE)
    {    
      VoltageSetter_SetVoltage((uint32_t)(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE - 1)); 
    }
    else if (setResistance > 0)
    {
      uint64_t voltage = ((((uint64_t)measurementValues->unfilteredVoltage)) * setResistance) / 1000;

      if (voltage >= (uint64_t)VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE)
      {
        VoltageSetter_SetVoltage((uint32_t)(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE - 1)); 
      }
      else
      {
        VoltageSetter_SetVoltage((uint32_t)voltage);
      }
    }
    else
    {
      // Set zero voltage   
      VoltageSetter_SetVoltage(0);
    }
  }
}

void Control_KeepResistanceCV(void)
{
  static Control_VoltageActions lastAction = Control_VoltageDown;
  
  if (measurementValues->milliseconds - measurementTimer > CONTROL_BANDWIDTH_LIMIT_CV)
  {    
    if (setResistance >= VOLTMETER_INPUT_RESISTANCE)
    {
      VoltageSetter_SetVoltage((uint32_t)(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE - 1));
    }
    else if (setResistance > 0)
    {            
      Control_SWCV(setResistance, &lastResistance, measurementValues->unfilteredResistance, &lastAction);
    }        
    else
    {
      VoltageSetter_SetVoltage(0);
    } 
    measurementTimer = measurementValues->milliseconds;    
  }  
  VoltageSetter_Do();
}

void Control_SetVoltageSoftware(void)
{
  stepSize = 0;
  Control_LimitCurrentStepSize(&stepSize);
  lastVoltage = measurementValues->unfilteredVoltage;
  VoltageSetter_SetVoltage(lastVoltage);

  if (lastVoltage < (uint32_t)VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE)
  { 
    VoltageSetter_SetVoltage(lastVoltage); // Set the last measured voltage as the initial estimate
  }
  else
  {
    VoltageSetter_SetVoltage((uint32_t)(VOLTAGE_SETTER_MAXIMUM_HIVOLTAGE - 1)); 
  }
}

void Control_KeepVoltageSoftware(void)
{
  static Control_CurrentActions lastAction = Control_CurrentUp;
  
  if (measurementValues->milliseconds - measurementTimer > CONTROL_BANDWIDTH_LIMIT_CC)
  {    
    if (setVoltage == 0)
    {
      CurrentSetter_SetCurrent((uint32_t)(CURRENT_SETTER_MAXIMUM_HICURRENT - 1));
    }
    else if (measurementValues->unfilteredVoltage < VOLTMETER_THRESHOLD_VOLTAGE)
    {
      CurrentSetter_SetCurrent(0);
    }
    else
    {            
      Control_SWCC(setVoltage, &lastVoltage, measurementValues->unfilteredVoltage, &lastAction);
    }   
         
    measurementTimer = measurementValues->milliseconds;
  }  
  CurrentSetter_Do();
}

void Control_SetMPPT(void)
{  
  lastPower = measurementValues->unfilteredPower;
  lastLastPower = measurementValues->unfilteredPower;

  stepSize = 0;
  Control_LimitVoltageStepSize(&stepSize);

  if (setVoltage == 0) // zero set voltage will attempt to initialize automatically
  {
    CurrentSetter_SetCurrent(0);
    CurrentSetter_Do();
    MPPT_initialized = false;
  }
  else
  {
    VoltageSetter_SetVoltage(setVoltage);
    VoltageSetter_Do();
    MPPT_initialized = true;
  }
  measurementCounter = measurementValues->counter;
}

void Control_KeepMPPT(void)
{
  static Control_VoltageActions MPPTAction = Control_VoltageDown;
  static Control_VoltageActions lastMPPTAction = Control_VoltageDown;
  static Control_VoltageActions action;

  // initialization
  if (!MPPT_initialized)
  {
    if (measurementValues->counter == (uint8_t)(measurementCounter + 5))
    {      
      VoltageSetter_SetVoltage((measurementValues->unfilteredVoltage * 9) / 10); // set 90% of open-circuit voltage
      VoltageSetter_Do();
      measurementCounter = measurementValues->counter;
      MPPT_initialized = true;
    }
    else
    {
      CurrentSetter_Do();
    }
    return;
  }

  // main loop
  if (measurementValues->milliseconds - measurementTimer > CONTROL_BANDWIDTH_LIMIT_CV)
  { 
    action = MPPTAction;
    if (measurementValues->unfilteredVoltage < VOLTMETER_THRESHOLD_VOLTAGE) /* Increase voltage on zero voltage */
    {
      stepSize = 0;
      Control_LimitVoltageStepSize(&stepSize);
      MPPTAction = Control_VoltageUp;
    }
    else if (measurementValues->unfilteredCurrent < AMMETER_THRESHOLD_VOLTAGE) /* Decrease voltage on zero current */
    {
      stepSize = 0;
      Control_LimitVoltageStepSize(&stepSize);
      MPPTAction = Control_VoltageDown;
    }
    else if (MPPTAction != lastMPPTAction) /* Different former actions - choose the one that led to more favourable outcome */
    {
      if ((measurementValues->unfilteredPower / 2 + lastLastPower / 2) < lastPower)
      {
        MPPTAction = lastMPPTAction; 
      }
    }
    else if (lastPower > measurementValues->unfilteredPower) /* Both former action were the same,
                                                                then judge if the last one was efficient or not.
                                                                If the previous power was larger, reverse the action,
                                                                otherwise stay with the current course */
    {
      if (MPPTAction == Control_VoltageUp)
      {
        MPPTAction = Control_VoltageDown;
      }
      else
      {
        MPPTAction = Control_VoltageUp;
      }
    }
    
    /* Now perform the computed action */
    if (MPPTAction == lastMPPTAction) /* Increase step size when action did not change - dynamic step size */
    {
      Control_StepSizeVoltagePlus();
    }
    else /* Decrease step size when action is reversed */
    {
      Control_StepSizeVoltageMinus();
    }  
    
    if (MPPTAction == Control_VoltageUp)
    {        
      VoltageSetter_Plus(stepSize); 
    }
    else
    {
      VoltageSetter_Minus(stepSize); 
    }
    
    lastMPPTAction = action;
    lastLastPower = lastPower;
    lastPower = measurementValues->unfilteredPower;
    measurementTimer = measurementValues->milliseconds;
  }
  VoltageSetter_Do();  
}

//void Control_KeepMPPT(void)
//{
//  static Control_CurrentActions MPPTAction = Control_CurrentUp;
//  static Control_CurrentActions lastMPPTAction = Control_CurrentUp;
//  static Control_CurrentActions action;
//
//  if (MPPT_initialized == 0)
//  {
//    if (measurementValues->counter == measurementCounter + 10)
//    {      
//      VoltageSetter_SetVoltage((measurementValues->voltage * 9) / 10);
//      VoltageSetter_Do();    
//      measurementCounter = measurementValues->counter;
//      MPPT_initialized = 1;
//    }
//    else
//    {
//      CurrentSetter_Do();
//    }
//    return;
//  }
//  else if (MPPT_initialized == 1)
//  {
//    if (measurementValues->counter == measurementCounter + 10)
//    {
//      MPPT_initialized = 2;
//      CurrentSetter_SetCurrent(measurementValues->current);
//      measurementCounter = measurementValues->counter;
//      CurrentSetter_Do();
//    }
//    else
//    {       
//      VoltageSetter_Do();         
//    }
//    return;  
//  }
//  
//  if (measurementCounter != measurementValues->counter)
//  { 
//    action = MPPTAction;
//    if (measurementValues->current == 0) /* Increase current on zero current */
//    {
//      stepSize = CONTROL_MINIMUM_CURRENT_STEP;
//      MPPTAction = Control_CurrentUp;
//    }
//    else if (measurementValues->voltage == 0) /* Decrease current on zero voltage */
//    {
//      stepSize = CONTROL_MINIMUM_CURRENT_STEP;
//      MPPTAction = Control_CurrentDown;
//    }
//    else if (MPPTAction != lastMPPTAction) /* Different former actions - choose the one that led to more favourable outcome */
//    {
//      if ((measurementValues->power / 2 + lastLastPower / 2) < lastPower)
//      {
//        MPPTAction = lastMPPTAction; 
//      }
//    }
//    else if (lastPower > measurementValues->power) /* Both former action were the same,
//                                                      then judge if the last one was efficient or not.
//                                                      If the previous power was larger, reverse the action,
//                                                      otherwise stay with the current course */
//    {
//      if (MPPTAction == Control_CurrentUp)
//      {
//        MPPTAction = Control_CurrentDown;
//      }
//      else
//      {
//        MPPTAction = Control_CurrentUp;
//      }
//    }
//    
//    /* Now perform the computed action */
//    if (MPPTAction == lastMPPTAction) /* Increase step size when action did not change - dynamic step size */
//    {
//      Control_StepSizeCurrentPlus();
//    }
//    else /* Decrease step size when action is reversed */
//    {
//      Control_StepSizeCurrentMinus();
//    }  
//    
//    if (MPPTAction == Control_CurrentUp)
//    {        
//      CurrentSetter_Plus(stepSize); 
//    }
//    else
//    {
//      CurrentSetter_Minus(stepSize); 
//    }
//    
//    lastMPPTAction = action;
//    lastLastPower = lastPower;
//    lastPower = measurementValues->power;
//    measurementCounter = measurementValues->counter;
//  }
//  CurrentSetter_Do();  
//}

void Control_SetMaxCurrent(void)
{
  CurrentSetter_SetMaxCurrentThisRange();
}

void Control_SetCCCV(Control_CCCVStates state)
{
  switch (state)
  {
    case Control_CCCV_CC:
      digitalWrite(CONTROL_CCCV_PIN, LOW);
      cccvState = state;
    break;
    case Control_CCCV_CV:
      digitalWrite(CONTROL_CCCV_PIN, HIGH);
      cccvState = state;
    break;
    case Control_CCCV_CC_SimpleAmmeter:
      digitalWrite(CONTROL_CCCV_PIN, LOW);
      cccvState = state;
    break;
    default:
    break;
  }
}

void Control_StepSizeCurrentPlus(void)
{ 
  /* Increase step size */
  stepSize = stepSize + (stepSize >> 2) + 1;
  Control_LimitCurrentStepSize(&stepSize);
}

void Control_StepSizeCurrentMinus(void)
{
  stepSize = stepSize >> 1;
  Control_LimitCurrentStepSize(&stepSize);
}

void Control_LimitCurrentStepSize(uint32_t * stepSize)
{
  if (stepSize == NULL)
  {
    return;
  }
  
  /* Get limit according to current range */
  RangeSwitcher_CurrentRanges currentRange = RangeSwitcher_GetCurrentRange();
  uint32_t maximumStep = currentRange == CurrentRange_HighCurrent ? CONTROL_MAXIMUM_HI_CURRENT_STEP : CONTROL_MAXIMUM_LO_CURRENT_STEP;
  uint32_t minimumStep = currentRange == CurrentRange_HighCurrent ? CONTROL_MINIMUM_HI_CURRENT_STEP : CONTROL_MINIMUM_LO_CURRENT_STEP;

  /* Limit relative step size to under 40% of the measured current value */
  if (*stepSize > (measurementValues->unfilteredCurrent << 1) / 5 ) // maximum step size is 40% of the measured value
  {
    *stepSize = (measurementValues->unfilteredCurrent << 1) / 5;
  }

  /* Limit relative step size to above 0.1% of the measured current value */
  if (*stepSize < measurementValues->unfilteredCurrent / 1000 ) // minimum step size is 0.1% of the measured value
  {
    *stepSize = measurementValues->unfilteredCurrent / 1000;
  }

  /* Limit step size to under the defined maximum (different for each current range) */
  if (*stepSize > maximumStep)
  {
    *stepSize = maximumStep;
  }

  /* Limit step size to above the defined minimum (different for each current range) */
  if (*stepSize < minimumStep)
  {
    *stepSize = minimumStep;
  }
}

void Control_StepSizeVoltagePlus(void)
{
  RangeSwitcher_VoltageRanges voltageRange = RangeSwitcher_GetVoltageRange();
  uint32_t minimumStep = voltageRange == VoltageRange_HighVoltage ? CONTROL_MINIMUM_HI_VOLTAGE_STEP : CONTROL_MINIMUM_LO_VOLTAGE_STEP;  
  stepSize += minimumStep;
  Control_LimitVoltageStepSize(&stepSize);
}

void Control_StepSizeVoltageMinus(void)
{
  RangeSwitcher_VoltageRanges voltageRange = RangeSwitcher_GetVoltageRange();
  uint32_t minimumStep = voltageRange == VoltageRange_HighVoltage ? CONTROL_MINIMUM_HI_VOLTAGE_STEP : CONTROL_MINIMUM_LO_VOLTAGE_STEP;
  stepSize -= minimumStep;
  Control_LimitVoltageStepSize(&stepSize);
}

void Control_LimitVoltageStepSize(uint32_t * stepSize)
{
  if (stepSize == NULL)
  {
    return;
  }
  
  /* Get limit according to voltage range */
  RangeSwitcher_VoltageRanges voltageRange = RangeSwitcher_GetVoltageRange();
  uint32_t maximumStep = voltageRange == VoltageRange_HighVoltage ? CONTROL_MAXIMUM_HI_VOLTAGE_STEP : CONTROL_MAXIMUM_LO_VOLTAGE_STEP;
  uint32_t minimumStep = voltageRange == VoltageRange_HighVoltage ? CONTROL_MINIMUM_HI_VOLTAGE_STEP : CONTROL_MINIMUM_LO_VOLTAGE_STEP;

  /* Limit relative step size to under 40% of the measured voltage value */
  if (*stepSize > (measurementValues->unfilteredVoltage << 1) / 5 ) // maximum step size is 40% of the measured value
  {
    *stepSize = (measurementValues->unfilteredVoltage << 1) / 5;
  }

  /* Limit relative step size to above 0.1% of the measured voltage value */
  if (*stepSize < measurementValues->unfilteredVoltage / 1000 ) // minimum step size is 0.1% of the measured value
  {
    *stepSize = measurementValues->unfilteredVoltage / 1000;
  }

  /* Limit step size to under the defined maximum (different for each voltage range) */
  if (*stepSize > maximumStep)
  {
    *stepSize = maximumStep;
  }

  /* Limit step size to above the defined minimum (different for each voltage range) */
  if (*stepSize < minimumStep)
  {
    *stepSize = minimumStep;
  }
}

void Control_SWCC(uint32_t setValue, uint32_t * lastValue, uint32_t presentValue, Control_CurrentActions * lastAction)
{
  uint32_t lastDifference, presentDifference;
  Control_CurrentActions action;
  
  /* Calculate absolute values of difference between set value (target) and present value */
  if (setValue > *lastValue)
  {
    lastDifference = setValue - *lastValue;
  }
  else
  {
    lastDifference = *lastValue - setValue;
  }      
  if (setValue > presentValue)
  {
    presentDifference = setValue - presentValue;
  }
  else
  {
    presentDifference = presentValue - setValue;
  }
  
  /* If the last action led to favourable outcome, keep it. Otherwise, reverse the action. */
  if (presentDifference <= lastDifference)
  {
    action = *lastAction;     
    Control_StepSizeCurrentPlus();
  }
  else
  {
    if (*lastAction == Control_CurrentDown)
    {
      action = Control_CurrentUp;
    }
    else
    {
      action = Control_CurrentDown;
    }    
    Control_StepSizeCurrentMinus();     
  }
  
  /* Perform the computed action */
  if (action == Control_CurrentDown)
  {
    CurrentSetter_Minus(((uint64_t)stepSize * lastDifference) / setValue);
  }
  else
  {
    CurrentSetter_Plus(((uint64_t)stepSize * lastDifference) / setValue);
  }    
  
  /* Assign last values */
  *lastAction = action;
  *lastValue = presentValue;
}

void Control_SWCV(uint32_t setValue, uint32_t * lastValue, uint32_t presentValue, Control_VoltageActions * lastAction)
{
  uint32_t lastDifference, presentDifference;
  Control_VoltageActions action;
  
  /* Calculate absolute values of difference between set value (target) and present value */
  if (setValue > *lastValue)
  {
    lastDifference = setValue - *lastValue;
  }
  else
  {
    lastDifference = *lastValue - setValue;
  }      
  if (setValue > presentValue)
  {
    presentDifference = setValue - presentValue;
  }
  else
  {
    presentDifference = presentValue - setValue;
  }
  
  /* If the last action led to favourable outcome, keep it. Otherwise, reverse the action. */
  if (presentDifference <= lastDifference)
  {
    action = *lastAction;     
    Control_StepSizeVoltagePlus();
  }
  else
  {
    if (*lastAction == Control_VoltageDown)
    {
      action = Control_VoltageUp;
    }
    else
    {
      action = Control_VoltageDown;
    }    
    Control_StepSizeVoltageMinus();     
  }
  
  /* Perform the computed action */
  if (action == Control_VoltageDown)
  {
    VoltageSetter_Minus(stepSize);
  }
  else
  {
    VoltageSetter_Plus(stepSize);
  }    
  
  /* Assign last values */
  *lastAction = action;
  *lastValue = presentValue;
}

Control_CCCVStates Control_GetCCCV(void)
{
  return cccvState;
}

const ErrorMessaging_Error * Control_GetError(void)
{
  return &ControlError;
}

/* </Implementations> */ 
