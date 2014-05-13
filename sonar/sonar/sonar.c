
#define F_CPU 2000000UL
#define ENABLE_UART_D0  1
#define D0_BAUD			115200
#define D0_CLK2X		0

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "uart.h"

char str[256];
uint16_t afstandcm;

uint16_t totaalklokslagen;
uint8_t echocheck;
uint8_t puls = 1;

void input_init(void)
{
	PORTC.PIN2CTRL = PORT_ISC_BOTHEDGES_gc;
	PORTC.DIRCLR = PIN2_bm;
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTC_PIN2_gc;
	TCC0.CTRLD = TC_EVACT_CAPT_gc |	TC_EVSEL_CH0_gc;
	TCC0.CTRLB = TC0_CCAEN_bm;
	TCC0.CTRLA = TC_CLKSEL_DIV2_gc;
	TCC0.INTCTRLB = TC_CCAINTLVL_LO_gc;
	TCC0.PER = 0xFFFF;
}

void timerd_init(void)
{
	TCD0.CTRLB     = TC_WGMODE_NORMAL_gc;
	TCD0.CTRLA     = TC_CLKSEL_DIV64_gc;
	TCD0.INTCTRLA  = TC_OVFINTLVL_LO_gc;
	TCD0.PER       = 3000;
}

ISR(TCC0_CCA_vect)
{
	totaalklokslagen = TCC0.CCA;
	afstandcm = totaalklokslagen/116*2;
	TCC0.CTRLFSET = TC_CMD_RESTART_gc;
}

ISR(TCD0_OVF_vect)
{
	puls = 1;
}

int main(void)
{

	init_uart(&uartD0, &USARTD0, F_CPU, D0_BAUD, D0_CLK2X);
	timerd_init();
	input_init();

	PORTE.DIRSET = PIN0_bm;
	PORTC.DIRSET = PIN0_bm;
	PORTC.DIRCLR = PIN2_bm;
	
	PMIC.CTRL     |= PMIC_LOLVLEN_bm;
	sei();

	while(1){
	
		if ( puls == 1 ) {
		PORTC.OUTSET = PIN0_bm;
		_delay_us(10);
		PORTC.OUTCLR = PIN0_bm;	
		puls = 0;
		}
	
		sprintf(str, " %d cm, %d klokslagen \n\r ", afstandcm, totaalklokslagen);
		uart_puts(&uartD0, str);
		_delay_ms(100);
		
	}

}