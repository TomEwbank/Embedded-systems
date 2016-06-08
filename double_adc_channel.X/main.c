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

#define PING_RECEIVED_LOW 475
#define PING_RECEIVED_HIGH 545

#define MODE_UART 0
#define MODE_RECV 1 

#define TMR2_PR_32KHZ 127

#define LED_PORT1 LATAbits.LATA0
#define LED_PORT2 LATAbits.LATA1
#define LED_PORT3 LATAbits.LATA2
#define LED_PORT4 LATAbits.LATA3
#define LED_PORT5 LATAbits.LATA4

/**
 * Mode of the program
 */
volatile bool read_adc = true;

/**
 * ADC fields
 */
volatile int adc_val1 = 514;
volatile int adc_val2 = 514;
volatile bool ping1 = false;
volatile bool ping2 = false;
volatile bool sensor1_turn = true;
volatile int counter = 0;

//interrupt vector
void interrupt interrupt_service_routine(void) {
    
    if (TMR0IE == 1 && TMR0IF == 1) {
        ++counter;
        if (counter == 200) {
            LED_PORT3 = !LED_PORT3;
            counter = 0;
            TMR0ON = 0;
            TMR2ON = 1;
            LED_PORT1 = 0;
            LED_PORT2 = 0;
            ping1 = false;
            ping2 = false;
            //read_adc = true;
        }
        TMR0IF = 0;
    }
    
    // Timer2 overflow
    if (TMR2IE && TMR2IF) { 
        TMR2IF = 0;
        ADCON0bits.GO = 1;
    }
    
    // ADC conversion finished
    if (PIE1bits.ADIE && PIR1bits.ADIF) {
        PIR1bits.ADIF = 0; // reset end of conversion flag
        if (sensor1_turn) {
            adc_val1 = (ADRESH << 2) | (ADRESL >> 6);
            
            if (adc_val1 < PING_RECEIVED_LOW || adc_val1 > PING_RECEIVED_HIGH) {
                    TMR2ON = 0;
                    //read_adc = 0;
                    ping1 = true;
                    LED_PORT1 = 1;
                    TMR0ON = 1; // enables timer0
            }
            
            sensor1_turn = false;
            ADCON0bits.CHS = 0b01011; // Use RB4 as input channel
            ADCON0bits.GO = 1;
        } else { 
            adc_val2 = (ADRESH << 2) | (ADRESL >> 6);
            
            if (adc_val2 < PING_RECEIVED_LOW || adc_val2 > PING_RECEIVED_HIGH) {
                    TMR2ON = 0;
                    //read_adc = 0;
                    ping2 = true;
                    LED_PORT2 = 1;
                    TMR0ON = 1; // enables timer0
                    
            }
            
            ADCON0bits.CHS = 0b01001; // Use RB3 as input channel
            sensor1_turn = true;
        }
        
    }
}

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
    ANSELBbits.ANSB4 = 1; // Set RB4 as analog input
    TRISBbits.TRISB4 = 1; // Set RB4 pin as input
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
static void initLed() {
    TRISA = 0; // Port RA = output
    LATA = 0; // Clear RA outputs (set 0V)
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

void startTMR2() {
    // TIMER2: f_out = ~30kHz
    // Period : 127
    PR2 = 191;
    OpenTimer2(TIMER_INT_ON 
                & T2_PS_1_1
                & T2_POST_1_1);
}

void main(void){
    initOscillator();
    initLed();
    initADCON();
    initUART();
    startTMR2();
    
    // TIMER0: f_out = 61Hz, interrupt every 16ms 
    TMR0H = 0x00;
    TMR0L = 0x00; // counter preload
    T08BIT = 0; // 16bit mode
    T0CS = 0; // Internal clock (Fosc/4))
    T0SE = 0; // Increment on low-to-high transition
    PSA = 1; // No prescaler assigned
    TMR0ON = 0; // enables timer0
    TMR0IE = 1; // enables timer0 interrupts
    
    // enable interrupts
    GIEH = 1;
    PEIE = 1;
    //int i = 0;
    
    while(1) {
        //if(read_adc) {
                // check thresholds
                PIE1bits.ADIE = 0;
                int val1 = ping1;
                int val2 = ping2;
                int val3 = adc_val1;
                int val4 = adc_val2;
                PIE1bits.ADIE = 1;
               
                if (val1) {
                    send_debug("PING 1");
                }
                if (val2) {
                    send_debug("PING 2");
                }
                
                send_coord(val3,val4);    
        //}
        
    } 
}
