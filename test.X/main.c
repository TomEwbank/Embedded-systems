/*
 * File:   main.c
 * Author: tom
 *
 * Created on 10 mai 2016, 0:08
 */


#include "p18f24k50.h"
#include <xc.h>
#include <timers.h>

#pragma config FOSC = INTOSCIO
#pragma config MCLRE = ON
#pragma config PBADEN = OFF
#pragma config LVP = OFF
#pragma config WDTEN = OFF, DEBUG = OFF

volatile int counter = 0;

//interrupt vector
void interrupt MyIntVec(void) {
//    if (TMR0IE == 1 && TMR0IF == 1) {
//                  
//        if (LATB4) {         
//            LATB4 = 0;
//        } else {
//            LATB4 = 1;
//        }
//             
//        TMR0H = 0xF0;
//        TMR0L = 0x60; 
//        TMR0IF = 0;
//    }
    
//    if (TMR2IE && TMR2IF) {
//        
//        LATB5 = !LATB5;
////        if (LATB5) {         
////            LATB5 = 0;
////        } else {
////            LATB5 = 1;
////        }
//             
//       
//        TMR2IF = 0;
//    }
    
    // ADC conversion finished
//    if (PIE1bits.ADIE && PIR1bits.ADIF) {
//        PIR1bits.ADIF = 0; // reset end of conversion flag
//        ++counter;
//        if (counter == 10000) {
//            LATA0 = !LATA0;
//            counter = 0;
//        }
//        ADCON0bits.GO = 1;
//             
//    }
    if (TMR1IE && TMR1IF) {
        LATA1 = !LATA1;
        TMR1IF = 0;
    }
    
}


static void initOscillator() {
    OSCCONbits.IDLEN = 1; // Device enters in sleep mode
    OSCCONbits.IRCF = 0b111; // Internal oscillator set to 16MHz
    OSCCONbits.OSTS = 0; // Running from internal oscillator
    OSCCONbits.HFIOFS = 0; // Frequency not stable
    OSCCONbits.SCS = 0b00; // Primary clock defined by FOSC<3:0>
}

static void outputPWM() {
    TRISC1 = 0;  // set PORTC as output, RC1 is the pwm pin output
    PORTC = 0;   // clear PORTC
    //PR2 = 0b01100011;
    
    /* PWM registers configuration
    * Fosc = 16000000 Hz
    * Fpwm = 40000.00 Hz (Requested : 40000 Hz)
    * Duty Cycle = 50 %
    * Resolution is 8 bits
    * Prescaler is 4
    * Ensure that your PWM pin is configured as digital output
    * see more details on http://www.micro-examples.com/
    * this source code is provided 'as is',
    * use it at your own risks
    */
    PR2 = 0b00011000; // period value
    CCP2CON = 0b00011100;
    CCPR2L = 0b00001100;
    T2CKPS1 = 0;    // Prescaler value - High bit
    T2CKPS0 = 1;    // Prescaler value - Low bit
    TMR2ON = 1;     // Activate timer 2
  
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

void main(void) {
    
    initOscillator();
    initADCON();
    //outputPWM();
    
    TRISA = 0; // PORTB = output
    LATA = 0; // Clear B outputs (set 0V)-- pas plutot 0???

    
//    // TIMER0: f_out = 1kHz, interrupt every 1ms 
//    // Reload for TMR0H:TMR0L : 61536
//    TMR0H = 0xF0;
//    TMR0L = 0x60; // counter preload
//    T08BIT = 0; // 16bit mode
//    T0CS = 0; // Internal clock (Fosc/4))
//    T0SE = 0; // Increment on low-to-high transition
//    PSA = 1; // No prescaler assigned
//    TMR0ON = 1; // enables timer0
//    TMR0IE = 1; // enables timer0 interrupts
    
//    PR2 = 0b00000100 ;
//    T2CON = 0b00000101 ;
//    TMR2IE = 1;
    
    //
    
//    OpenTimer0(TIMER_INT_ON     // Timer enabled
//                & T0_8BIT      // Timer counter on 16 bits
//                & T0_SOURCE_INT // Internal clock as source
//                & T0_EDGE_RISE
//                & T0_PS_1_1);   // Timer divider 1:1
    
    
//    // TIMER1: f_out = 1kHz, interrupt every 1ms 
//    // Reload for TMR0H:TMR0L : 61536
//    TMR1H = 0xF0;
//    TMR1L = 0x60;
//    OpenTimer1(TIMER_INT_ON         // Timer enabled
//                & T1_8BIT_RW
//                & T1_SOURCE_FOSC_4
//                & T1_PS_1_1         // Timer divider PRE 1:1 
//                & T1_OSC1EN_OFF
//                & T1_SYNC_EXT_OFF,
//               
//               TIMER_GATE_OFF
//                & TIMER_GATE_POL_HI
//                & TIMER_GATE_TOGGLE_OFF
//                & TIMER_GATE_SRC_T1GPIN
//                & TIMER_GATE_INT_OFF);
    
    OpenTimer1(TIMER_INT_ON         // Timer enabled
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
       
    GIEH = 1; // Enable global Interrupt
    PEIE = 1; // Enable peripheral interrupts
    
//    outputPWM();
    ADCON0bits.GO = 1;
    while(1){}

}
