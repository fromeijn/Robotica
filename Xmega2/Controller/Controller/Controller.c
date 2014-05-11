/*
 * Controller.c
 *
 * Created: 8-5-2014 14:28:26
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


int main(void)
{
	PORTE.DIRSET = PIN0_bm;
	
	init_uart(&uartD0, &USARTD0, F_CPU, D0_BAUD, D0_CLK2X);
	PMIC.CTRL = PMIC_LOLVLEN_bm;
	sei();
	
	
	uart_puts(&uartD0, "init succeeded\n");
	_delay_ms(1000);
	
    while(1)
    {
        PORTE.OUTTGL = PIN0_bm;
        uart_puts(&uartD0, "hallo\n");
        _delay_ms(1000); 
    }
}