#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/fpu.h"  
#include "driverlib/timer.h"
#include "Filter.h"

// hier noch Ihren Filterheader einbinden

// Praeprozessor-Makros
#define SAMPLERATE 44000
#define FILTERORDER 50

// Funktionen-Deklarationen
void adcIntHandler(void);
void setup(void);
// hier nach Bedarf noch weitere Funktionsdeklarationen einfuegen

const uint8_t HIGH = 0xFF;    // LED an
const uint8_t LOW = 0;        // LED aus
const uint32_t ENERGY_MAX = 16777216; // 4096 squared = 16777216, 4096 because adc has 12 bits

// global variables
int32_t bufferSample[FILTERORDER];
int32_t sampleIndex = 0;
// hier nach Bedarf noch weitere globale Variablen einfuegen

float energy = 0;
float filteredValue = 0;
void main(void) // nicht veraendern!! Bitte Code in adcIntHandler einfuegen 
{
    setup();
    while(1){}
}

void setup(void){// konfiguriert den Mikrocontroller

    // konfiguriere System-Clock
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    uint32_t period = SysCtlClockGet()/SAMPLERATE;

    // aktiviere Peripherie
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // aktiviere Gleitkommazahlen-Modul
    FPUEnable();
    FPUStackingEnable();
    FPULazyStackingEnable();
    FPUFlushToZeroModeSet(FPU_FLUSH_TO_ZERO_EN);
	
	// konfiguriere GPIO
    GPIOPinTypeADC(GPIO_PORTE_BASE,GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);

    // konfiguriere Timer
    TimerConfigure(TIMER0_BASE,TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, period - 1);
    TimerControlTrigger(TIMER0_BASE,TIMER_A,true);
    TimerEnable(TIMER0_BASE,TIMER_A);

    // konfiguriere ADC
    ADCClockConfigSet(ADC0_BASE,ADC_CLOCK_RATE_FULL,1);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH1|ADC_CTL_IE|ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntClear(ADC0_BASE,3);
    ADCIntRegister(ADC0_BASE,3,adcIntHandler);
    ADCIntEnable(ADC0_BASE,3);

}

void adcIntHandler(void){
   // Bitte Code hier einfuegen
    uint32_t adcInputValue;
    ADCSequenceDataGet(ADC0_BASE,3,&adcInputValue);

    //damit nur letzter Wert berechnet wird
   if(sampleIndex <= 48){

       bufferSample[sampleIndex] = adcInputValue;
       sampleIndex++;

   }

   else if (sampleIndex == 49){

       bufferSample[sampleIndex] = adcInputValue;
       for(int i = 0; i < FILTERORDER; i++){
           filteredValue += bufferSample[FILTERORDER -1 -i] * B[i];
       }

       energy = filteredValue * filteredValue;

       sampleIndex = 0;


   }

   

   // am Ende von adcIntHandler, Interrupt-Flag loeschen
   ADCIntClear(ADC0_BASE,3);
}
