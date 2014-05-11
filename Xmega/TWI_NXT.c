/*
 * CFile1.c
 *
 * Created: 8-5-2014 13:15:42
 *  Author: Floris
 */
#define F_CPU     2000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define ENABLE_UART_D0  1
#define D0_BAUD			115200
#define D0_CLK2X		0

#include "uart.h"
#include "twi_slave_driver.h"

#define SLAVE_ADDRESS 0x01
TWI_Slave_t twiSlave;

char str[256];
char indata[16];
uint8_t index;

void TWIC_SlaveProcessData(void)
{
	index = twiSlave.bytesReceived;
	indata[index] = twiSlave.receivedData[index];
}

int main(void)
{
	PORTE.DIRSET = PIN0_bm;
	PORTE.OUTSET = PIN0_bm;
	
	TWI_SlaveInitializeDriver(&twiSlave, &TWIC, TWIC_SlaveProcessData);
	TWI_SlaveInitializeModule(&twiSlave, SLAVE_ADDRESS, TWI_SLAVE_INTLVL_LO_gc);
	
	init_uart(&uartD0, &USARTD0, F_CPU, D0_BAUD, D0_CLK2X);
	PMIC.CTRL = PMIC_LOLVLEN_bm;
	sei();
	
	twiSlave.sendData[0] = 'H';
	twiSlave.sendData[1] = 'o';
	twiSlave.sendData[2] = 'i';
	while(1)
	{
		for (int x = 0; x<10; x++)
		{
			twiSlave.sendData[4] = '0'+x;
			uart_putc(&uartD0, '0'+x);
			uart_putc(&uartD0, '\n');
			uart_putc(&uartD0, '\r');
			sprintf(str, "byte = 0x%02x\n\r", indata[index]);
			uart_puts(&uartD0, str);
			_delay_ms(1000);
		}
	}
}

ISR(TWIC_TWIS_vect)
{
	TWI_SlaveInterruptHandler(&twiSlave);
	PORTE.OUTTGL = PIN0_bm;
}