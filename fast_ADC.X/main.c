/*
 * File:   main.c
 * Author: tom
 *
 * Created on 8 juin 2016, 17:06
 */


/*
 * File:   main.c
 * Author: tom
 *
 * Created on 6 juin 2016, 13:32
 */

//#include <xc.h>
#include <p30f1010.h>

#pragma config FNOSC = FRC_PLL
#pragma config FRANGE = FRC_HI_RANGE

#define TIMER_PERIOD 0x0064 /* Set the timer period for 3.43 usec*/
int main(void)
{
    OSCTUN = 0b0111;
    
    /* Set up the ADC Module */
    ADCONbits.ADSIDL = 0; /* Operate in Idle Mode*/
    ADCONbits.FORM = 0; /* Output in Integer Format*/
    ADCONbits.EIE = 0; /* No Early Interrupt*/
    ADCONbits.ORDER = 0; /* Even channel first*/
    ADCONbits.SEQSAMP = 0; /* Sequential Sampling Enabled*/
    ADCONbits.ADCS = 0b010; /* Clock Divider is set up for Fadc/14*/
    ADPCFG = 0xFFFC; /* AN0 and AN1 are analog inputs.*/
    ADSTAT = 0; /* Clear the ADSTAT register*/
    ADCPC0bits.TRGSRC0 = 1; /* Trigger conversion on Timer 1 Period Match*/
    ADCPC0bits.IRQEN0 = 1; /* Enable the interrupt*/
    /* Set up Timer1 */
    T1CON = 0; /* Timer with 0 prescale*/
    TMR1 = 0; /* Clear the Timer counter*/
    PR1 = TIMER_PERIOD; /* Load the period register*/
    /* Set up the Interrupts */
    IFS0bits.ADIF = 0; /* Clear AD Interrupt Flag*/
    IPC2bits.ADIP = 4; /* Set ADC Interrupt Priority*/
    IEC0bits.ADIE = 1; /* Enable the ADC Interrupt*/
    
    /* Enable ADC */
    ADCONbits.ADON = 1; /* Start the ADC module*/

    ADPCFGbits.PCFG3 = 1;
    TRISBbits.TRISB3 = 0;
    LATBbits.LATB3 = 0;

        ADCPC0bits.SWTRG0 = 1;

    while(1);
}

void __attribute__ ((__interrupt__)) _ADCInterrupt(void)
{
    /* AD Conversion complete interrupt handler */
//    int channel0Result, channel1Result;
    
    IFS0bits.ADIF = 0; /* Clear ADC Interrupt Flag*/
    //channel0Result = ADCBUF0; /* Get the conversion result*/
    //channel1Result = ADCBUF1;
    ADCPC0bits.SWTRG0 = 1;
    LATBbits.LATB3 = !LATBbits.LATB3;
    ADSTATbits.P0RDY= 0; /* Clear the ADSTAT bits*/
    
}