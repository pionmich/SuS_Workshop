#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/fpu.h"

// Praepozessor-Makros
#define SAMPLERATE 44000
#define DFT_LIMIT(threshold_value) ((threshold_value * DFT_MAX)/8)   //Maximaler Grenzwert der Skala in Achteln. -> Exponentielle Skala

//Funktionen-Deklarationen
void adcIntHandler(void);
void setup(void);
// hier nach Bedarf noch weitere Funktionsdeklarationen einfuegen
float maxValue(float maxArray[]);

// globale Variablen
const float DoublePi = 6.283185308;
const uint16_t DFT_SIZE = 440;
const uint16_t DFT_MAX = 4000;
const uint8_t HIGH = 0xFF;    // LED an
const uint8_t LOW = 0;        // LED aus
int32_t bufferSample[440];
float bufferDFT[440];
float maxArray[440];
uint16_t index = 0;




void main(void){ // nicht veraendern!! Bitte Code in adcIntHandler einfuegen 
    setup();
    while(1){}
}

void setup(void){//konfiguriert den Mikrocontroller

    // konfiguriere SystemClock
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
// todo: HIER MUSS DER INDEX DES HOECHSTEN WERTES AUSGEGEBEN WERDEN
float maxValue(float maxArray[])
{
    uint16_t k=0;
    float max= 0.0;
    max = maxArray[0];
    for(k=1 ; k<DFT_SIZE; k++)
    {
        if(maxArray[k]>max)
        {
            max = maxArray[k];
        }
    }
    return max;
}
void adcIntHandler(void){
    // Variablen initialisieren
    float dftRe = 0.0;
    float dftIm = 0.0;
    uint16_t n = 0;
    uint16_t j = 0;
    float absDFT = 0.0;
    float maxDFT = 0.0;

    uint32_t adcInputValue;
    ADCSequenceDataGet(ADC0_BASE,3,&adcInputValue);

    // ADC Werte im Buffer speichern
    bufferSample[index] = adcInputValue;

    if (index == DFT_SIZE -1)
    {
        for (j = 0; j< DFT_SIZE; j++)
        {

            for (n = 0; n < DFT_SIZE; n++)
            {
                dftRe = dftRe + bufferSample[n] * cosf(- DoublePi*j*n/DFT_SIZE);
                dftIm = dftIm + bufferSample[n] * sinf(- DoublePi*j*n/DFT_SIZE);
            }
            dftRe = dftRe / 440;
            dftIm = dftIm / 440;
            absDFT = sqrtf(dftRe * dftRe + dftIm * dftIm);
            bufferDFT[j] = absDFT;
        }

        maxDFT = maxValue(bufferDFT) *100;  //mal 100, da frequenzauflösung von 100


        if ((maxDFT >= 0 )&&(maxDFT < DFT_LIMIT(1)) ) //DFT_LIMIT(x) = DFT_MAX * (x/8)
        {
            GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_0) , HIGH ) ;
            GPIOPinWrite (GPIO_PORTB_BASE, (
                    GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                    GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                    GPIO_PIN_7) , LOW ) ;
        }
        else if ((maxDFT >= (DFT_LIMIT(1)) )&&(maxDFT < DFT_LIMIT(2)) ) //DFT_LIMIT(x) = DFT_MAX * (x/8)
        {
            GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_1) , HIGH ) ;
            GPIOPinWrite (GPIO_PORTB_BASE, (
                    GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 |
                    GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                    GPIO_PIN_7) , LOW ) ;
        }
        else if ((maxDFT >= (DFT_LIMIT(2)) )&&(maxDFT < DFT_LIMIT(3)) ) //DFT_LIMIT(x) = DFT_MAX * (x/8)
        {
            GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_2) , HIGH ) ;
            GPIOPinWrite (GPIO_PORTB_BASE, (
                    GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_3 |
                    GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                    GPIO_PIN_7) , LOW ) ;
        }
        else if ((maxDFT >= (DFT_LIMIT(3)) )&&(maxDFT < DFT_LIMIT(4)) ) //DFT_LIMIT(x) = DFT_MAX * (x/8)
        {
            GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_3) , HIGH ) ;
            GPIOPinWrite (GPIO_PORTB_BASE, (
                    GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_0 |
                    GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                    GPIO_PIN_7) , LOW ) ;
        }
        else if ((maxDFT >= (DFT_LIMIT(4)) )&&(maxDFT < DFT_LIMIT(5)) ) //DFT_LIMIT(x) = DFT_MAX * (x/8)
        {
            GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_4) , HIGH ) ;
            GPIOPinWrite (GPIO_PORTB_BASE, (
                    GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                    GPIO_PIN_0 | GPIO_PIN_5 | GPIO_PIN_6 |
                    GPIO_PIN_7) , LOW ) ;
        }
        else if ((maxDFT >= (DFT_LIMIT(5)) )&&(maxDFT < DFT_LIMIT(6)) ) //DFT_LIMIT(x) = DFT_MAX * (x/8)
        {
            GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_5) , HIGH ) ;
            GPIOPinWrite (GPIO_PORTB_BASE, (
                    GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                    GPIO_PIN_4 | GPIO_PIN_0 | GPIO_PIN_6 |
                    GPIO_PIN_7) , LOW ) ;
        }
        else if ((maxDFT >= (DFT_LIMIT(6)) )&&(maxDFT < DFT_LIMIT(7)) ) //DFT_LIMIT(x) = DFT_MAX * (x/8)
        {
            GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_6) , HIGH ) ;
            GPIOPinWrite (GPIO_PORTB_BASE, (
                    GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                    GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_0 |
                    GPIO_PIN_7) , LOW ) ;
        }
        else if ((maxDFT >= (DFT_LIMIT(7)) )&&(maxDFT <= DFT_LIMIT(8)) ) //DFT_LIMIT(x) = DFT_MAX * (x/8)
        {
            GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_7) , HIGH ) ;
            GPIOPinWrite (GPIO_PORTB_BASE, (
                    GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                    GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                    GPIO_PIN_0) , LOW ) ;
        }
    }

    // Index erhoehen, wenn voll auf 0 zurücksetzen
    index = (index + 1) % 440;

    // am Ende von adcIntHandler, Interrupt-Flag loeschen
    ADCIntClear(ADC0_BASE,3);
}
