/*
 *  LOCALIZATION SYSTEM
 * 
 * File:   main.c
 * Author: tom
 *
 * Created on 8 juin 2016, 17:06
 */


//#include <xc.h>
#include <p30f1010.h>

#include <libpic30.h>
#include <timer.h>
#include <outcompare.h> //for PWM

#pragma config FNOSC = FRC_PLL
#pragma config FRANGE = FRC_HI_RANGE

#define PING_RECEIVED_LOW 413
#define PING_RECEIVED_HIGH 571
#define PWM_PERIOD_PMR2 0x1FE


volatile int i = 0;
volatile int t = 300;
volatile int received = 0;


static void enableTimer2PWM() {
    TMR2 = 0; // Clear count register
    
    PR2 = PWM_PERIOD_PMR2; // Period
    
    // Default config
    T2CONbits.TSIDL = 0; // Continue timer when CPU idle
    T2CONbits.TGATE = 0; // Timer gated time accumulation disabled
    T2CONbits.T32 = 0; // 16-bit timer
    T2CONbits.TCS = 0; // Internal clock
    
    // Prescale -> 1:1
    T2CONbits.TCKPS = 0b00;
    
    // Activate
    T2CONbits.TON = 1;
}

static void disableTimer2() {
    T2CONbits.TON = 0;
    CloseTimer2();
}

static void initPWM() {    
    OC1RS = 0xFE; // PWM duty cycle -> Should be PR2/2 for 50%
    OC1CONbits.OCTSEL = 0; //Use Timer 2 as clock source
    OC1CONbits.OCM = 0b110; // PWM mode on OC1, Fault pin disabled
}

static void startPWM() {
    // Enable the timer
    enableTimer2PWM();
}

static void stopPWM() {
    disableTimer2();
}





int main(void)
{
    OSCTUN = 0b0111;
    
    initPWM();
    startPWM();
    
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
    PR1 = 0xFFFF; //TIMER_PERIOD; /* Load the period register*/
    T1CONbits.TON = 0;
    IEC0bits.T1IE = 1; // Enables timer1 interrupts
    
    /* Set up the Interrupts */
    IFS0bits.ADIF = 0; /* Clear AD Interrupt Flag*/
    IPC2bits.ADIP = 4; /* Set ADC Interrupt Priority*/
    IEC0bits.ADIE = 1; /* Enable the ADC Interrupt*/
    
    /* Enable ADC */
    ADCONbits.ADON = 1; /* Start the ADC module*/

    ADPCFGbits.PCFG2 = 1;
    TRISBbits.TRISB2 = 0;
    LATBbits.LATB2 = 0;
    
    ADPCFGbits.PCFG3 = 1;
    TRISBbits.TRISB3 = 0;
    LATBbits.LATB3 = 0;

    ADCPC0bits.SWTRG0 = 1;

    while(1);
}

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void) {
    if (i == t) {
        received = 0;
        ADCPC0bits.SWTRG0 = 1;
        T1CONbits.TON = 0;
        T1CON = 0; /* Timer with 0 prescale*/
        TMR1 = 0; /* Clear the Timer counter*/
        i = 0;
        
    } else {
        ++i;
    }
//    LATBbits.LATB2 = !LATBbits.LATB2;
    IFS0bits.T1IF = 0;
}

void __attribute__ ((__interrupt__)) _ADCInterrupt(void)
{
    /* AD Conversion complete interrupt handler */
    int sensor1_val, sensor2_val;
    
    IFS0bits.ADIF = 0; /* Clear ADC Interrupt Flag*/
    sensor1_val = ADCBUF0; /* Get the conversion result*/
    sensor2_val = ADCBUF1;
//    ADCPC0bits.SWTRG0 = 1;
    
//    LATBbits.LATB2 = !LATBbits.LATB2;
    
    if (sensor1_val < PING_RECEIVED_LOW || sensor1_val > PING_RECEIVED_HIGH) {
        LATBbits.LATB2 = 1;
        received = 1;
    } else {
        LATBbits.LATB2 = 0;
    }
    if (sensor2_val < PING_RECEIVED_LOW || sensor2_val > PING_RECEIVED_HIGH) {
        LATBbits.LATB3 = 1;
        received = 1;
    } else {
        LATBbits.LATB3 = 0;       
    }
    
    if (!received) {
       ADCPC0bits.SWTRG0 = 1;

    } else {
       T1CONbits.TON = 1;
    }
    
//    LATBbits.LATB2 = !LATBbits.LATB2;
    ADSTATbits.P0RDY= 0; /* Clear the ADSTAT bits*/
    
}