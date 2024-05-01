#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

// Praeprozessor-Makros
#define BUFFER_SIZE 1000
#define SAMPLERATE 44000


// Funktionen-Deklarationen
void adcIntHandler(void);
void setup(void);

// Konstanten
const uint8_t HIGH = 0xFF;    // LED an
const uint8_t LOW = 0;        // LED aus
const uint32_t VALUE_MAX = 16777216; // 4096 squared = 16777216, 4096 because adc has 12 bits

// globale Variablen
// hier die benötigten globalen Variablen, wie den Ringbuffer einfuegen
uint32_t ringBuffer[BUFFER_SIZE];    //array mit n=BUFFER_SIZE Elementen
uint32_t writeIndex = 0;
uint32_t readIndex = 0;
uint32_t i = 0;
#define AVERAGE_MAX VALUE_MAX / BUFFER_SIZE
#define AVERAGE_POT_MAX (10^ (AVERAGE_MAX))  // maximaler Grenzwert der Skala
#define AVERAGE_POT(threshold_value) (10^ ((threshold_value * AVERAGE_MAX)/8))   //Maximaler Grenzwert der Skala in Achteln


void main(void){ // nicht veraendern!! Bitte Code in adcIntHandler einfuegen
    setup();

    while(1){}
}

void setup(void){// konfiguriert den MiKrocontroller
    // konfiguriere System-Clock
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    uint32_t period = SysCtlClockGet()/SAMPLERATE;

    // aktiviere Peripherie
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

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
// Funktionen aus Wikipedia
void put (uint32_t item)  //Funktion setzt einen neuen Wert in den Buffer
{
    //  if ((writeIndex + 1) % BUFFER_SIZE == readIndex)
    //  {
    //     // buffer is full, #ERROR
    //  }
    ringBuffer[writeIndex] = item;
    writeIndex = (writeIndex + 1) % BUFFER_SIZE;

}


uint32_t get ()   //Funktion holt den naechsten Wert aus dem Buffer
{
    //  if (readIndex == writeIndex)
    //  {
    //     // buffer is empty #ERROR
    //     return 0;
    //  }

    uint32_t item = ringBuffer[readIndex];
    readIndex = (readIndex + 1) % BUFFER_SIZE;
    return item;
}
// Ende Funktionen aus Wikipedia
void adcIntHandler (void){
    uint32_t adcInputValue;
    ADCSequenceDataGet(ADC0_BASE,3,&adcInputValue);
    // Bitte Code hier einfuegen
    uint32_t currentValue = adcInputValue * adcInputValue;    //aktuellen Wert auslesen und quadrieren
    put (currentValue);
    float average = 0.0;
    for (i = 0; i <= BUFFER_SIZE; i++)
    {

        uint32_t value = get();
        average = ((average + value)/ BUFFER_SIZE);
    }
    if ((average >= (AVERAGE_POT(0)) )&&(average < AVERAGE_POT(1)) ) //AVERAGE_POT(x) = AVERAGE_POT_MAX * (x/8) // AVERAGE_POT(0) = 0
    {
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_0) , HIGH ) ;
        GPIOPinWrite (GPIO_PORTB_BASE, (
                GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                GPIO_PIN_7) , LOW ) ;
    }
    else if ((average >= (AVERAGE_POT(1)) )&&(average < AVERAGE_POT(2)) ) //AVERAGE_POT(x) = AVERAGE_POT_MAX * (x/8) // AVERAGE_POT(0) = 0
    {
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_0 |
                GPIO_PIN_1) , HIGH ) ;
        GPIOPinWrite (GPIO_PORTB_BASE, ( GPIO_PIN_2 | GPIO_PIN_3 |
                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                GPIO_PIN_7) , LOW ) ;
    }
    else if ((average >= (AVERAGE_POT(2)) )&&(average < AVERAGE_POT(3)) ) //AVERAGE_POT(x) = AVERAGE_POT_MAX * (x/8) // AVERAGE_POT(0) = 0
    {
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_0 |
                GPIO_PIN_1 | GPIO_PIN_2) , HIGH ) ;
        GPIOPinWrite (GPIO_PORTB_BASE, ( GPIO_PIN_3 |
                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                GPIO_PIN_7) , LOW ) ;
    }
    else if ((average >= (AVERAGE_POT(3)) )&&(average < AVERAGE_POT(4)) ) //AVERAGE_POT(x) = AVERAGE_POT_MAX * (x/8) // AVERAGE_POT(0) = 0
    {
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_0 |
                GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3) , HIGH ) ;
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                GPIO_PIN_7) , LOW ) ;
    }
    else if ((average >= (AVERAGE_POT(4)) )&&(average < AVERAGE_POT(5)) ) //AVERAGE_POT(x) = AVERAGE_POT_MAX * (x/8) // AVERAGE_POT(0) = 0
    {
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_0 |
                GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                GPIO_PIN_4) , HIGH ) ;
        GPIOPinWrite (GPIO_PORTB_BASE, ( GPIO_PIN_5 | GPIO_PIN_6 |
                GPIO_PIN_7) , LOW ) ;
    }
    else if ((average >= (AVERAGE_POT(5)) )&&(average < AVERAGE_POT(6)) ) //AVERAGE_POT(x) = AVERAGE_POT_MAX * (x/8) // AVERAGE_POT(0) = 0
    {
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_0 |
                GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                GPIO_PIN_4 | GPIO_PIN_5) , HIGH ) ;
        GPIOPinWrite (GPIO_PORTB_BASE, ( GPIO_PIN_6 |
                GPIO_PIN_7) , LOW ) ;
    }
    else if ((average >= (AVERAGE_POT(6)) )&&(average < AVERAGE_POT(7)) ) //AVERAGE_POT(x) = AVERAGE_POT_MAX * (x/8) // AVERAGE_POT(0) = 0
    {
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_0 |
                GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6) , HIGH ) ;
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_7) , LOW ) ;
    }
    else if ((average >= (AVERAGE_POT(7)) )&&(average < AVERAGE_POT(8)) ) //AVERAGE_POT(x) = AVERAGE_POT_MAX * (x/8) // AVERAGE_POT(0) = 0
    {
        GPIOPinWrite (GPIO_PORTB_BASE, (GPIO_PIN_0 |
                GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                GPIO_PIN_7) , HIGH ) ;
    }




    // am Ende von adcIntHandler, Interrupt-Flag loeschen
    ADCIntClear(ADC0_BASE,3);
}
