
#define PWM_PERIOD_PMR2 0x1F0

#include <xc.h>
#include <libpic30.h>
#include <timer.h>
#include <outcompare.h> //for PWM

#pragma config FNOSC = FRC_PLL
#pragma config FRANGE = FRC_HI_RANGE


//void __attribute__((__interrupt__, __auto_psv__)) _T2Interrupt(void) {
//    LATBbits.LATB3 = !LATBbits.LATB3;
//    IFS0bits.T2IF = 0;
//}

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

int main(int argc, char** argv)
{
    // Pin configuration
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB3 = 0;
    ADPCFGbits.PCFG2 = 1;
    ADPCFGbits.PCFG3 = 1;
    
    TRISDbits.TRISD0 = 0;
    
    //EnableIntT2; // Enables Interrupt of timer 2
    
    initPWM();
    startPWM();
    
    while(1) {}
    
    return 0;
}
