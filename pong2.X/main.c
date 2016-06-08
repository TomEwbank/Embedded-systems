/*
 * File:   main.c
 *
 * Created on August 16, 2010, 12:09 PM
 */

#include "p18f24k50.h"
#include <xc.h>
#include "custom_timer.h"
#include "protocol.h"
#include <timers.h>
#include <stdbool.h>
#include <stdio.h>
 
#pragma config FOSC = INTOSCIO
#pragma config MCLRE = ON
#pragma config PBADEN = OFF
#pragma config LVP = OFF
#pragma config WDTEN = OFF, DEBUG = OFF

#define PING_RECEIVED_LOW 464
#define PING_RECEIVED_HIGH 564

#define MODE_RECV 1 
#define MODE_WAITING_BEFORE_SEND 2
#define MODE_SENDING 3
#define MODE_WAITING_AFTER_SEND 4

#define SEND_DURATION 1
#define WAIT_DURATION 15

#define LED_PORT1 LATAbits.LATA0
#define LED_PORT2 LATAbits.LATA1

/**
 * Mode of the program
 */
volatile char mode = MODE_RECV;
volatile int counter = 0;

volatile bool send_ping = true;
volatile bool ping_send = false;
volatile bool blocking_sensors = false;
volatile bool listening = false;

volatile int tmr0_counter = 0;

volatile bool RTT_received = false;
volatile char no_RTT_counter = false; 
volatile bool notSentToUART = true;

volatile char sensor1_RTT_low = 0;
volatile char sensor1_RTT_high = 0;

/**
 * ADC fields
 */
//volatile int adc_val = -1;
//volatile bool adc_new_value = false;


/**
 * Init the ADC to read values from RB3
 */
static void initADCON() {
    // Init ADCON0
    ADCON0bits.ADON = 1; // Enable ADC module
    ADCON0bits.GO_nDONE = 0; // Reset GO to 0
    ADCON0bits.CHS = 0b01001; // Use RB3 as input channel
    ANSELBbits.ANSB3 = 1; // Set RB3 as analog input
    TRISBbits.TRISB3 = 1; // Set RB3 pin as input
    // Init ADCON1
    ADCON1bits.TRIGSEL = 0; // special trigger from CCP2
    ADCON1bits.PVCFG = 0; // connect reference Vref+ to internal Vdd
    ADCON1bits.NVCFG = 0; // connect reference Vref- to external Vdd
    // Init ADCON2
    ADCON2bits.ADFM = 0; // result format is left justified
    ADCON2bits.ACQT = 0b100; // 8 TAD
    ADCON2bits.ADCS = 0b101; // Fosc / 16
    // Enable ADC interrupts
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
}

/**
 * Initialize RA0 to be a digital output 
 */
static void initLedPorts() {
    TRISAbits.TRISA0 = 0; // Port RA0 = output
    LATAbits.LATA0 = 0; // Clear RA0 output (set 0V)
    TRISAbits.TRISA1 = 0; // Port RA1 = output 
    LATAbits.LATA1 = 0; // Clear RA1 output (set 0V)
}

/**
 * Init oscillator to be internal and 16MHz
 */
static void initOscillator() {
    OSCCONbits.IDLEN = 1; // Device enters in sleep mode
    OSCCONbits.IRCF = 0b111; // Internal oscillator set to 16MHz
    OSCCONbits.OSTS = 0; // Running from internal oscillator
    OSCCONbits.HFIOFS = 0; // Frequency not stable
    OSCCONbits.SCS = 0b00; // Primary clock defined by FOSC<3:0>
}

static inline void startADC() {
    ADCON0bits.GO = 1;
}

static void initPWM() {
    TRISC1 = 0;  // set PORTC as output, RC1 is the pwm pin output
    PORTC = 0;   // clear PORTC
    PR2 = 0b00011000 ;
    CCP2CON = 0b00011100;
    CCPR2L = 0b00001100;
    T2CKPS1 = 0;    // Prescaler value - High bit
    T2CKPS0 = 1;    // Prescaler value - Low bit
}


static void startPWM() {
    TRISC1 = 0;
    TMR2ON = 1;  
}


static void stopPWM() {
    // Stop PWM
    TMR2ON = 0;
    TRISC1 = 1;
}


static void initTMR1() {
    OpenTimer1(TIMER_INT_OFF         // Timer enabled
                & T1_16BIT_RW
                & T1_SOURCE_FOSC_4
                & T1_PS_1_1         // Timer divider PRE 1:1 
                & T1_OSC1EN_OFF
                & T1_SYNC_EXT_OFF,
               
               TIMER_GATE_OFF
                & TIMER_GATE_POL_HI
                & TIMER_GATE_TOGGLE_OFF
                & TIMER_GATE_SRC_T1GPIN
                & TIMER_GATE_INT_OFF);
}


static void startTimer1() {
    TMR1ON = 1;
}


static void stopTimer1() {
    TMR1ON = 0;
    TMR1L = 0;
    TMR1H = 0;
}



//interrupt vector
void interrupt interrupt_service_routine(void) {    
    // Timer0 overflow
    if (INTCONbits.TMR0IE && INTCONbits.TMR0IF) { 
        INTCONbits.TMR0IF = 0;
        
              
        if (listening) {
            if (tmr0_counter == 35) {
                tmr0_counter = 0;
                send_ping = true;
                listening = false;
  
                if (RTT_received) {
                    no_RTT_counter = 0;
                } else {
                    ++no_RTT_counter;
                }

                RTT_received = false;
                notSentToUART = true;
                ADIF = 0;
                ADIE = 0;
            } else {
                ++tmr0_counter;
            }
        } else if (ping_send) { 
            stopPWM();
            ping_send = false;
            blocking_sensors = true;
        } else if (send_ping) {
            startTimer1();
       
            startPWM(); // Sent for 1ms
            ping_send = true;
            send_ping = false;
        } else if (blocking_sensors && tmr0_counter == 15) { // Blocked during 15ms
            tmr0_counter = 0;
            blocking_sensors = 0;
            listening = 1;
            ADIE = 1;
            startADC();
        } else {
            ++tmr0_counter;
        }
        startTMR0For1kHz();
    }
    
    // ADC conversion finished
    if (PIE1bits.ADIE && PIR1bits.ADIF) {
        PIR1bits.ADIF = 0; // reset end of conversion flag
        int sensor1_val = (ADRESH << 2) | (ADRESL >> 6);
        LED_PORT2 = !LED_PORT2;
        if (sensor1_val < PING_RECEIVED_LOW || sensor1_val > PING_RECEIVED_HIGH) {
            sensor1_RTT_low = TMR1L;
            sensor1_RTT_high = TMR1H;
            stopTimer1();
            RTT_received = true;
            ADIE = 0;
            
        }
         startADC();
    }
    
    
}


void main(void){
    initOscillator();
    initTMR0();
    initTMR1();
    initLedPorts();
    initADCON();
    initUART();
    initPWM();
    
    int fetched_sensor1_RTT = -1;
    
    // enable interrupts
    GIEH = 1;
    PEIE = 1;
    
    bool readyForUART = false;
    //int c = 0;
    
    startTMR0For1kHz();
    
    while(1) {
        if (no_RTT_counter > 3 || no_RTT_counter > 3) {
            //send_debug("Tracker not found");
        } else {
            
            if (RTT_received && notSentToUART) {
                fetched_sensor1_RTT = (sensor1_RTT_high<<8) & sensor1_RTT_low;
                readyForUART = true;
            }
            
            if (readyForUART && notSentToUART) {
                send_coord(fetched_sensor1_RTT, fetched_sensor1_RTT);
                readyForUART = false;
                notSentToUART = false;
            }
//            ++c;
//            if (c == 10000) {
//                LED_PORT1 = !LED_PORT1;
//                c = 0;
//            }
        }
    } 
}
