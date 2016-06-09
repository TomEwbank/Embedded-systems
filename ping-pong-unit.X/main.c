/*
 *  PING-PONG SYSTEM
 * 
 * File:   main.c
 * Author: tom
 *
 * Created on 9 juin 2016, 16:47
 */


//#include <xc.h>
#include <p30f1010.h>

#include <libpic30.h>
#include <timer.h>
#include <outcompare.h> //for PWM
#include <stdbool.h>

#pragma config FNOSC = FRC_PLL
#pragma config FRANGE = FRC_HI_RANGE

#define PING_RECEIVED_LOW 413
#define PING_RECEIVED_HIGH 571
#define PWM_PERIOD 0x1FE
#define TIMER1_PERIOD 20000 // 1ms timer

#define MODE_RECV 1 
#define MODE_WAITING_BEFORE_SEND 2
#define MODE_SENDING 3
#define MODE_WAITING_AFTER_SEND 4

#define MODE_WAIT1_DURATION 15
#define MODE_SEND_DURATION 16
#define MODE_WAIT2_DURATION 31

volatile char mode = MODE_RECV;
volatile int timer1_counter = 0;

static void initTimer1() {
    /* Set up Timer1 */
    T1CON = 0; /* Timer with 0 prescale*/
    TMR1 = 0; /* Clear the Timer counter*/
    PR1 = TIMER1_PERIOD; /* Load the period register*/
    IEC0bits.T1IE = 1; // Enables timer1 interrupts
}

static void startTimer1() {
    TMR1 = 0; /* Clear the Timer counter*/
    IEC0bits.T1IE = 1; // Enables timer1 interrupts
    T1CONbits.TON = 1;
}

static void initTimer2() {
    // Default config
    T2CONbits.TSIDL = 0; // Continue timer when CPU idle
    T2CONbits.TGATE = 0; // Timer gated time accumulation disabled
    T2CONbits.T32 = 0; // 16-bit timer
    T2CONbits.TCS = 0; // Internal clock
    
    // Prescale -> 1:1
    T2CONbits.TCKPS = 0b00;
}

static void initPWM() {    
    OC1RS = 0xFE; // PWM duty cycle -> Should be PR2/2 for 50%
    OC1CONbits.OCTSEL = 0; //Use Timer 2 as clock source   
}

static void startPWM() {
    TMR2 = 0; // Clear count register
    PR2 = PWM_PERIOD; // Period
    OC1CONbits.OCM = 0b110; // PWM mode on OC1, Fault pin disabled 
    
    // Activate 
    T2CONbits.TON = 1;
}

static void stopPWM() {
    CloseTimer2();
    OC1CONbits.OCM = 0; // PWM output disabled
}

void initADC() {
    /* Set up the ADC Module */
    ADCONbits.ADSIDL = 0; /* Operate in Idle Mode*/
    ADCONbits.FORM = 0; /* Output in Integer Format*/
    ADCONbits.EIE = 0; /* No Early Interrupt*/
    ADCONbits.ORDER = 0; /* Even channel first*/
    ADCONbits.SEQSAMP = 0; /* Sequential Sampling Disabled */
    ADCONbits.ADCS = 0b010; /* Clock Divider is set up for Fadc/8 */
    ADPCFG = 0xFFFC; /* AN0 and AN1 are analog inputs.*/
    ADSTAT = 0; /* Clear the ADSTAT register*/
    ADCPC0bits.TRGSRC0 = 1; /* Trigger conversion with software */
    ADCPC0bits.IRQEN0 = 1; /* Enable the interrupt */
    
    /* Set up the Interrupts */
    IFS0bits.ADIF = 0; /* Clear AD Interrupt Flag*/
    IPC2bits.ADIP = 4; /* Set ADC Interrupt Priority*/
    IEC0bits.ADIE = 1; /* Enable the ADC Interrupt*/
}

static void initLedOutputs() {
    /* Set up led outputs */
    ADPCFGbits.PCFG2 = 1;
    TRISBbits.TRISB2 = 0;
    LATBbits.LATB2 = 0;
    
    ADPCFGbits.PCFG3 = 1;
    TRISBbits.TRISB3 = 0;
    LATBbits.LATB3 = 0;
}

int main(void)
{
    OSCTUN = 0b0111; // Tune internal FRC: 9.7MHz+3% to have 10MHz => 20MIPS
    
    initPWM();
    initTimer2();
    initADC();
    initTimer1(); 
    initLedOutputs();
      
    /* Enable ADC */
    ADCONbits.ADON = 1; /* Start the ADC module*/
    
    /* Enable Timer 1 */
    T1CONbits.TON = 1;  
    
    IEC0bits.ADIE = 1;
    ADCPC0bits.SWTRG0 = 1;

    while(1) {
        
    }
}

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void) {

    IFS0bits.T1IF = 0;
      
    switch(mode) {
        case MODE_WAITING_BEFORE_SEND:
            if (timer1_counter >= MODE_WAIT1_DURATION) {
                LATBbits.LATB3 = !LATBbits.LATB3;
                mode = MODE_SENDING;
                startPWM();
            }
            break;
        case MODE_SENDING: 
            if (timer1_counter >= MODE_SEND_DURATION) {
                stopPWM();
                mode = MODE_WAITING_AFTER_SEND;
            }
            break;
        case MODE_WAITING_AFTER_SEND:
            if (timer1_counter >= MODE_WAIT2_DURATION) {
                CloseTimer1();
                mode = MODE_RECV;

                ADCPC0bits.SWTRG0 = 1;
            }
    }
    
    timer1_counter++;

    
    
}

void __attribute__ ((__interrupt__)) _ADCInterrupt(void)
{
 
    /* AD Conversion complete interrupt handler */
    int sensor1_val;
    
    IFS0bits.ADIF = 0; /* Clear ADC Interrupt Flag*/
    
    sensor1_val = ADCBUF0; /* Get the conversion result*/
    
    
    
    LATBbits.LATB2 = !LATBbits.LATB2;
    
    if (sensor1_val < PING_RECEIVED_LOW || sensor1_val > PING_RECEIVED_HIGH) {

        mode = MODE_WAITING_BEFORE_SEND;
        timer1_counter = 0;
        startTimer1();
        LATBbits.LATB3 = !LATBbits.LATB3;
    } 
    else {
        /* Keep sampling */
        ADCPC0bits.SWTRG0 = 1;
    }
    
    ADSTATbits.P0RDY= 0; /* Clear the ADSTAT bits*/
    
}

