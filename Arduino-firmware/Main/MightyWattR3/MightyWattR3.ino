/**
 * IMPORTANT!
 * Navigate to Configuration.h and select your Arduino version and ADC type
 */

#include "MightyWatt.h"
#include <math.h>
#include <Wire.h>

#define CYCLE_COUNTER_ENABLE       false

#if (CYCLE_COUNTER_ENABLE == true)
#include "Configuration.h"
#include "Measurement.h"
const static Measurement_Values * measurementValues;
static uint8_t measurementValues_Counter = 0;
#endif

void setup() 
{  
  delay(20); /* delay to give the hardware some time to stabilize */  
  Wire.begin();   
  Watchdog_Init(); /* system watchdog */
  MightyWatt_Init();
  delay(10); /* delay after init to give the hardware some time to stabilize */  

  #if (CYCLE_COUNTER_ENABLE == true)
    measurementValues = Measurement_GetValues();
  #endif
}

void loop() 
{ 
  Watchdog_Reset(); /* system watchdog reset */    
  MightyWatt_Do();

  #if (CYCLE_COUNTER_ENABLE == true)
    static uint32_t measurementCycleCounter = 0;
    static uint32_t loopCycleCounter = 0;
    static uint32_t lastUpdate = 0;
    uint32_t now = millis();
    static bool toggle = false;
  
    if ((now - lastUpdate) > 10000)
    {         
      SerialPort.print("Meas/s: ");
      SerialPort.print((measurementCycleCounter * 1000UL) / (now - lastUpdate));
      SerialPort.print("\tMain loop/s: ");
      SerialPort.println((loopCycleCounter * 1000UL) / (now - lastUpdate));
      measurementCycleCounter = 0;
      loopCycleCounter = 0;
      lastUpdate = now;     
    }
    if (measurementValues_Counter != measurementValues->counter)
    {
      measurementCycleCounter++;  
      measurementValues_Counter = measurementValues->counter;
    }    
    loopCycleCounter++;
  #endif
}

static void Watchdog_Init(void)
{
  #ifdef ZERO
    /* do not use NVM fuses */
    
    /* clock enable */
    GCLK->GENDIV.reg = 2 | (16 << 8); /* generic clock 2 with division 16 - this will create 8-second timeout on WDT PER = 0xB */
    GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(2); /* generator enable, source is OSCULP32K, generic clock generator 2 */
    while(GCLK->STATUS.bit.SYNCBUSY){} /* wait for sync */
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | 0x03; /* generic clock 2 enabled and routed to WDT */
  
    WDT->CONFIG.bit.PER = 0xB; /* 16384 clock cycles */
    while(WDT->STATUS.bit.SYNCBUSY){} /* wait for sync */
    WDT->CTRL.reg = (1 << 1); /* watchdog enable */
    while(WDT->STATUS.bit.SYNCBUSY){} /* wait for sync */
  	
    Watchdog_Reset();	
   #elif defined(UNO)
    cli();
    Watchdog_Reset();
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE) | (1 << WDP3) | (1 << WDP0);
    sei(); 
  #endif 
}

static void Watchdog_Reset(void)
{
  #ifdef ZERO
    WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;	
    while(WDT->STATUS.bit.SYNCBUSY){} /* wait for sync */
  #elif defined(UNO)
    asm("WDR");
  #endif 
}
