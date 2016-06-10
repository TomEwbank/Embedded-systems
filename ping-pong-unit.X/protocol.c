#include <p30f1010.h>
#include <libpic30.h>
#include <stdbool.h>
#include "protocol.h"

#define START_COORD 0x16
#define START_DEBUG 0x02
#define END_DEBUG 0x03

/**
 * Check whether the given char is reserved by the communication protocol
 */
static inline bool is_reserved(char c);
static inline void putc_when_ready(char c);
static void puts(const char* buffer);

static inline bool is_reserved(char c) {
	return c == START_DEBUG || c == START_COORD || c == END_DEBUG;
} 

static inline void putc_when_ready(char c) {
    while(!U1STAbits.TRMT);
    U1TXREG = c;
}

static void puts(const char* buffer) {
    unsigned int i;
    for (i = 0; buffer[i] != '\0'; ++i) {
        putc_when_ready(buffer[i]);
    }
}

void send_coord(unsigned int x_cm, unsigned int y_cm) {
	putc_when_ready(START_COORD);
	putc_when_ready((x_cm >> 8) & 0xFF);
	putc_when_ready((x_cm) & 0xFF);
	putc_when_ready((y_cm >> 8) & 0xFF);
	putc_when_ready((y_cm) & 0XFF); 
}

void send_debug_nchar(const char* buffer, unsigned int buffer_len) {
	putc_when_ready(START_DEBUG);
    unsigned int i;
	for (i = 0; i < buffer_len; ++i) {
		if (is_reserved(buffer[i])) { // prevent protocol to enter in invalid mode
			break;  
		}
		putc_when_ready(buffer[i]);
	}
	putc_when_ready(END_DEBUG);
}

void send_debug(const char* buffer) {
	putc_when_ready(START_DEBUG);
    puts(buffer);
	putc_when_ready(END_DEBUG);
}

void initUART() {
	// PIC30 19.5.3 in reference manual (page 511)
	// Baud rate: 115200 (so for Fcy = 20 MHz -> BRG 10)
    // Baud rate: 9600   (so for Fcy = 20 MHz -> BRG 129)
	U1BRG = 10; 

	U1MODEbits.PDSEL = 0; // number of data bits and parity selection
			  // 11 (9 bits, no parity), 10 (8 bits, odd parity), 
			  // 01 (8 bits, even parity), 00 (8 bits, no parity)
	U1MODEbits.STSEL = 0; // stop selection bit: 1 (2 stop bits), 0 (1 stop bits)

	IEC0bits.U1TXIE = 0; // DISABLE transmit interrupt
	IFS0bits.U1TXIF = 0; // Reset uart transmit interrupt flag

	U1MODEbits.UARTEN = 1; // Enable UART 
    U1STAbits.UTXEN = 1; // Enable transmission
	// PIC18 
	/*
	BRGH = 0; // low speed baud rate
    SPBRG = 25; //Writing SPBRG Register (baud rate: 9600)
    SYNC = 0; //Setting Asynchronous Mode, ie UART
    SPEN = 1; //Enables Serial Port
    TRISC7 = 1; //As Prescribed in Datasheet
    TRISC6 = 1; //As Prescribed in Datasheet
    LATC6 = 0;
    LATC7 = 0;
    CREN = 1; //Enables Continuous Reception
    TXEN = 1; //Enables Transmission
    */
}