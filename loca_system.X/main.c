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
#include <stdbool.h>
#include "protocol.h"
#include "track.h"

#pragma config FNOSC = FRC_PLL
#pragma config FRANGE = FRC_HI_RANGE

#define PING_RECEIVED_LOW 413
#define PING_RECEIVED_HIGH 571
#define PWM_PERIOD 0x1FE
#define RTT_PERIOD 0xFFFF
#define TIMER1_PERIOD 20000 // 1ms timer


//volatile int i = 0;
//volatile int t = 300;
//volatile int received = 0;

volatile bool send_ping = false;
volatile bool ping_send = false;
volatile bool blocking_sensors = false;
volatile bool listening = false;

volatile unsigned int timer1_counter = 0;
volatile unsigned int timer2_counter = 0;

volatile unsigned int RTT1_time = 0;
volatile unsigned int RTT2_time = 0;

volatile unsigned int RTT1_overflows = 0;
volatile unsigned int RTT2_overflows = 0;

volatile bool RTT1_received = false;
volatile bool RTT2_received = false;


static void initTimer1() {
    /* Set up Timer1 */
    T1CON = 0; /* Timer with 0 prescale*/
    TMR1 = 0; /* Clear the Timer counter*/
    PR1 = TIMER1_PERIOD; /* Load the period register*/
    IEC0bits.T1IE = 1; // Enables timer1 interrupts
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

static void startTimerRTT() {
    TMR2 = 0; // Clear count register
    PR2 = RTT_PERIOD; // Period
    
    // Activate
    IEC0bits.T2IE = 1; // Enables timer1 interrupts
    T2CONbits.TON = 1;
}

static void stopTimerRTT() {
    CloseTimer2();
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
    initUART();
    
    
    /* starting state */
    send_ping = true;
    
    
    /* Enable ADC */
    ADCONbits.ADON = 1; /* Start the ADC module*/
    
    /* Enable Timer 1 */
    T1CONbits.TON = 1;
    
    unsigned int fetched_RTT1_overflows, fetched_RTT2_overflows, fetched_RTT1_time, fetched_RTT2_time;
    unsigned int i = 0;
    Point point;
    point.x = 0;
    point.y = 0;
    point.z = 0;
    
    while(1) {
//        LATBbits.LATB2 = RTT1_received;
//        LATBbits.LATB3 = RTT2_received;
        
        IEC0bits.ADIE = 0;
        fetched_RTT1_overflows = RTT1_overflows;
        fetched_RTT2_overflows = RTT2_overflows;
        fetched_RTT1_time = RTT1_time;
        fetched_RTT2_time = RTT2_time;
        IEC0bits.ADIE = 1;
        
        ++i;
        if (i == 100) { 
            //LATBbits.LATB3 = !LATBbits.LATB3;
            unsigned int RTT1 = fetched_RTT1_overflows*3277 + (fetched_RTT1_time/20) - 16000;
            unsigned int RTT2 = fetched_RTT2_overflows*3277 + (fetched_RTT2_time/20) - 16000;
            int error = track(RTT1, RTT2, &point);
            if (error == 0) {
                send_debug("RTTs:");
                send_coord(RTT1, RTT2);
                send_debug("Point:");
                send_coord(point.x, point.y);
            } else {
                send_debug("BUG!");
                send_coord(error, 1);
            }

            i = 0;
        }
    }
}

void __attribute__((__interrupt__, __auto_psv__)) _T1Interrupt(void) {

    IFS0bits.T1IF = 0;
      
    if (listening) {
        
        if (timer1_counter == 50) {
            
            timer1_counter = 0;
            listening = false;
            send_ping = true;
            
            RTT1_received = false;
            RTT2_received = false;
            
            RTT1_overflows = 0;
            RTT2_overflows = 0;
            RTT1_time = 0;
            RTT2_time = 0;
            
            stopTimerRTT();
            timer2_counter = 0;
            
//            IEC0bits.ADIE = 0;
            IFS0bits.ADIF = 0; 
            ADSTATbits.P0RDY= 0;
               
        } else {
            
            ++timer1_counter;
        }
        
    } else if (ping_send) { 
        
        stopPWM();
        startTimerRTT();
        ping_send = false;
        blocking_sensors = true;
        
        
    } else if (send_ping) {
        
        initPWM();
        startPWM(); // Sent for 1ms
        send_ping = false;
        ping_send = true;
        
    } else if (blocking_sensors && timer1_counter == 15) { // Blocked during 15ms
        
        timer1_counter = 0;
        blocking_sensors = false;
        listening = true;
        
        
        
        /* Launch ADC */
//        IEC0bits.ADIE = 1;
        ADCPC0bits.SWTRG0 = 1;
     
    } else {
        ++timer1_counter;
    }
    
}

void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;   
    ++timer2_counter;
    
}

void __attribute__ ((__interrupt__)) _ADCInterrupt(void)
{
 
    /* AD Conversion complete interrupt handler */
    unsigned int sensor1_val, sensor2_val, time, overflows;
    
    IFS0bits.ADIF = 0; /* Clear ADC Interrupt Flag*/
    
    time = TMR2;
    overflows = timer2_counter;
    sensor1_val = ADCBUF0; /* Get the conversion result*/
    sensor2_val = ADCBUF1;
     
    ADSTATbits.P0RDY = 0; /* Clear the ADSTAT bits*/
    ADCPC0bits.SWTRG0 = (!RTT1_received || !RTT2_received);// ? 1 : 0;
    
    if (!RTT1_received && (sensor1_val < PING_RECEIVED_LOW || sensor1_val > PING_RECEIVED_HIGH)) {
        LATBbits.LATB2 = !LATBbits.LATB2;
        RTT1_overflows = overflows;
        RTT1_time = time;
        RTT1_received = true;
        
    } 
    
    if (!RTT2_received && (sensor2_val < PING_RECEIVED_LOW || sensor2_val > PING_RECEIVED_HIGH)) {
        LATBbits.LATB3 = !LATBbits.LATB3;
        RTT2_overflows = overflows;
        RTT2_time = time;
        RTT2_received = true;
    }    
    
}