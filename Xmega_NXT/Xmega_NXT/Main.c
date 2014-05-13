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
uint8_t transmitArray[256];

void TWIC_SlaveProcessData(void)
{
	uint8_t askbyte = twiSlave.receivedData[0];
	for(uint8_t i = 0; i < NUM_BYTES; i++)
	{
		if (transmitArray[i+askbyte] >= ' ')
		{
			twiSlave.sendData[i] = transmitArray[i+askbyte];
		}else{
			twiSlave.sendData[i] = '_';
		}
	}
	if(twiSlave.receivedData[0] == 0x01) transmitArray[5] = '_';
}
	

int main(void)
{
	PORTE.DIRSET = PIN0_bm;
	PORTE.DIRSET = PIN3_bm;
	
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
	TWI_SlaveInitializeModule(&twiSlave, SLAVE_ADDRESS, TWI_SLAVE_INTLVL_MED_gc);
	
	PMIC.CTRL = PMIC_LOLVLEN_bm|PMIC_MEDLVLEN_bm;
	sei();
	
	twiSlave.sendData[0] = 'T';
	twiSlave.sendData[1] = 'W';
	twiSlave.sendData[2] = 'I';

	
	sprintf(str, "UART Connected!!!\n\r");
	uart_puts(&uartD0, str);
	uint8_t x = 0;
	while(1)
	{
		transmitArray[0] = 'T';
		transmitArray[1] = 'W';
		transmitArray[2] = 'I';
		transmitArray[3] = ' ';
		transmitArray[4] = '0'+x;
		sprintf(str, "0x%d\n\r", x);
		uart_puts(&uartD0, str);
		x++;
		if (x>=10)
		{
			x = 0;
		}
		_delay_ms(1000);
	}
}

ISR(TWIC_TWIS_vect)
{
	TWI_SlaveInterruptHandler(&twiSlave);
	PORTE.OUTSET   = PIN0_bm;
	PORTE.OUTCLR  = PIN0_bm;
	
}

ISR(PORTC_INT0_vect)
{
	TCC1.CTRLA     = TC_CLKSEL_DIV1024_gc;
}

ISR(TCC1_OVF_vect)
{
	TCC1.CTRLA     = TC_CLKSEL_OFF_gc;
	int i = 0;
	while (i < NUM_BYTES)
	{
		if (USART_RXBufferData_Available(&uartC1))
		{
			receiveArray[i] = USART_RXBuffer_GetByte(&uartC1);
		}else{
			receiveArray[i] = '_';
		}
		i++;
	}
	
	i = 0;
	uart_puts(&uartD0, "ID: ");
	while (i < NUM_BYTES)
	{
		transmitArray[i+0x0F] = receiveArray[i];
		uart_putc(&uartD0, receiveArray[i]);
		i++;
	}
	transmitArray[5] = 'B';
}