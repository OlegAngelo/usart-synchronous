#include <xc.h>

#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

/* Goals : 
 *  1. transmit 8-bit data synchronously
 *  2. receive 8 bit data synchronously
 *  3. display received data to lcd
 * 
 * Other information:
 *  1. low speed
 *  2. Fosc = 4MHz
 *  3. Use interrupts for transmit and receive
*/

void delay(int time)
{
	int i, j;
	for (i = 0; i < time; i++)
	{
		for (j = 0; j < 100; j++);
	}
}

char to_send = 'A'; // 0x41

void interrupt ISR () {
    GIE = 0;

    if (TXIF) {
        TXREG = to_send;        
        // TXIF = 0;
        TXIE = 0;
        to_send++; // increment to test 'A', 'B', 'C', etc.
        if (to_send > 'Z') to_send = 'A';
    }

    GIE = 1;
}

void main () {
    // set as the usart pins
    TRISC6 = 0; // TX/CK as output (master provides clock)

    // Transmit config
    SYNC = 1; // sync mode
    SPEN = 1; // enable serial port
    TXEN = 1; // transmitter enabled
    TX9 = 0; // 8 bit transmission
    CSRC = 1; // sync master mode (clock generated internally from BRG)
    BRGH = 0; // low speed sync
    SPBRG = 103; // baud rate value baud = 9600 Fosc = 4Mhz

    // set interrupts
    TXIE = 1; // enable transmitter interrupt
    PEIE = 1; // peripheral intterupt enabled
    GIE = 1; // global interrupt enabled

    while (1) {
        while(!TRMT); // wait until TSR buffer is empty
        TXIE = 1; // trigger next interrupt
        delay(50);
    }
}