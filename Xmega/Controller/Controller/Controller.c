/*
 * Controller.c
 *
 *	RFID	Xmega
 *	3.3V	3.3V
 *	RST		C0
 *	GND		GND
 *	RO		NC
 *	MISO	C6
 *	MOSI	C5
 *	SCK		C7
 *	SDA		C4
 *
 * Created: 6-5-2014 19:46:14
 *  Author: Dedier
 */ 


#define F_CPU     2000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define ENABLE_UART_D0  1
#define D0_BAUD			115200
#define D0_CLK2X		0

#include "uart.h"
#include "MFRC552.h"
#include "spi.h"



int main(void)
{
	PORTE.DIRSET = PIN0_bm;
	
	init_uart(&uartD0, &USARTD0, F_CPU, D0_BAUD, D0_CLK2X);
	PMIC.CTRL = PMIC_LOLVLEN_bm;
	sei();
	
	spi_init();
	
	PORTC.DIRSET = chipSelectPin;
	PORTC.OUTCLR = chipSelectPin;
	PORTC.DIRSET = NRST;
	PORTC.OUTSET = NRST;
	
	MFRC522_Init();
		
	uart_puts(&uartD0, "init succeeded\n");
	_delay_ms(1000);
	
    while(1)
    {
        PORTE.OUTTGL = PIN0_bm;
		uart_puts(&uartD0, "hallo\n");
		_delay_ms(1000);
    }
	
}