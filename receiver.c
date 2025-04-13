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

unsigned char received_data = 0x00;

void delay(int time)
{
	int i, j;
	for (i = 0; i < time; i++)
	{
		for (j = 0; j < 100; j++);
	}
}

void instCtrl(unsigned char data)
{
	PORTD = data; // load data to port c since my port c is the output pins
	RB0 = 0;	  // set RS to 0 (instruction reg)
	RB1 = 0;	  // set RW to 0 (write)
	RB2 = 1;	  // set E to 1
	delay(5);
	RB2 = 0; // set E to 0 (strobe)
}

void initLCD()
{
	delay(50);

	instCtrl(0x3C); // function set: 8-bit; dual-line
	instCtrl(0x38); // display off
	instCtrl(0x01); // display clear
	instCtrl(0x06); // entry mode: increment; shift off
	instCtrl(0x0E); // display on; cursor off; blink off
}

void dataCtrl(unsigned char data)
{
	PORTD = data; // load data to PORT C since its our output
	RB0 = 1;	  // set RS to 1 (data reg)
	RB1 = 0;	  // set RW to 0 (write)
	RB2 = 1;	  // set E to 1
	delay(5);
	RB2 = 0; // set E to 0 (strobe)
}

void interrupt ISR () {
    GIE = 0;

    if (RCIF) {
        if (OERR) {
            CREN = 0;  // Clear error
            CREN = 1;  // Re-enable receiver
        }

        received_data = RCREG;   // Store received byte - must be read to clear RCIF
        RB3 = ~RB3; // toggle 

        // unsigned char lower = received_data & 0x0F;
        // unsigned char upper = (received_data >> 4) & 0x0F;

        dataCtrl('s');
        dataCtrl('t');
        dataCtrl(received_data);
        // dataCtrl(upper > 9 ? upper - 10 + 'A' : upper + '0');
        // dataCtrl(lower > 9 ? lower - 10 + 'A' : lower + '0');

        RCIF = 0;
        delay(200);
    }

    GIE = 1;
}

void main () {
    // lcd config - set port b & d as output
    TRISB = 0x00;
    TRISD = 0x00;

    initLCD();
    instCtrl(0x01);

    // set as the usart pins
    TRISC6 = 1; // CK (input in slave for shared clock)
    TRISC7 = 1; // RX/DT as input

    // receiver config
    SYNC = 1;   // Synchronous mode
    CSRC = 0; // sync master mode (clock generated externally from BRG in master(transmitter))
    SPEN = 1;   // Enable serial port
    TXEN = 0;   // Disable transmitter (receiver only)
    TX9 = 0; // 8 bit transmission
    BRGH = 0; // low speed sync
    // SPBRG = 103;  // Baud rate value baud= 9600 Fosc = 4MHz
    CREN = 1;   // Enable continuous receive

    // set interrupts
    RCIE = 1; // Enable receiver interrupt
    PEIE = 1; // Enable peripheral interrupts
    GIE = 1; // Enable global interrupts

    RB3 = 0;

    while (1) {
    }
}