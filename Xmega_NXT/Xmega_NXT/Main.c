#define F_CPU     2000000UL

#define ENABLE_UART_D0  1
#define D0_BAUD			115200
#define D0_CLK2X		0

#define ENABLE_UART_C1  1
#define C1_BAUD			9600
#define C1_CLK2X		0

#define NUM_BYTES 16
#define SLAVE_ADDRESS 0x01

//TX = Poort D 3 <Debug>
//RX = Poort D 2

//TX = Poort C 7
//RX = Poort C 6 <RFID>

#include "uart.h"
#include "twi_slave_driver.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

TWI_Slave_t twiSlave;

char str[256];
uint8_t receiveArray[NUM_BYTES];
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
	
	PORTC.INT0MASK = PIN6_bm;
	PORTC.PIN6CTRL = PORT_ISC_RISING_gc;
	PORTC.INTCTRL  = PORT_INT0LVL_LO_gc;
	
	init_uart(&uartD0, &USARTD0, F_CPU, D0_BAUD, D0_CLK2X);
	init_uart(&uartC1, &USARTC1, F_CPU, C1_BAUD, C1_CLK2X);
	
	TCC1.CTRLB     = TC_WGMODE_NORMAL_gc;
	TCC1.CTRLA     = TC_CLKSEL_OFF_gc;
	TCC1.INTCTRLA  = TC_OVFINTLVL_LO_gc;
	TCC1.PER       = 400;
	
	TWI_SlaveInitializeDriver(&twiSlave, &TWIC, TWIC_SlaveProcessData);
	TWI_SlaveInitializeModule(&twiSlave, SLAVE_ADDRESS, TWI_SLAVE_INTLVL_LO_gc);
	
	PMIC.CTRL = PMIC_LOLVLEN_bm;
	sei();
	
	twiSlave.sendData[0] = 'T';
	twiSlave.sendData[1] = 'W';
	twiSlave.sendData[2] = 'I';

	
	sprintf(str, "UART Connected!!!\n\r");
	uart_puts(&uartD0, str);
	while(1)
	{
		//nop
	}
}

ISR(TWIC_TWIS_vect)
{
	TWI_SlaveInterruptHandler(&twiSlave);
}

ISR(PORTC_INT0_vect)
{
	TCC1.CTRLA     = TC_CLKSEL_DIV1024_gc;
}

ISR(TCC1_OVF_vect)
{
	TCC1.CTRLA     = TC_CLKSEL_OFF_gc;
	PORTE.OUTSET   = PIN0_bm;
	int i = 0;
	while (i < NUM_BYTES)
	{
		if (USART_RXBufferData_Available(&uartC1))
		{
			receiveArray[i] = USART_RXBuffer_GetByte(&uartC1);
		}
		i++;
	}
	
	i = 0;
	uart_puts(&uartD0, "ID: ");
	while (i < NUM_BYTES)
	{
		if (receiveArray[i]>0)
		{
			if (i<11)
			{
				twiSlave.sendData[i] = receiveArray[i];
			}
			uart_putc(&uartD0, receiveArray[i]);
		}
		i++;
	}
	PORTE.OUTCLR   = PIN0_bm;
}