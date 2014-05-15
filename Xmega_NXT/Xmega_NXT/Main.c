#define F_CPU     2000000UL

#define ENABLE_UART_D0  1
#define D0_BAUD			115200
#define D0_CLK2X		0

#define ENABLE_UART_C1  1
#define C1_BAUD			9600
#define C1_CLK2X		0

#define NUM_BYTES 16
#define SLAVE_ADDRESS 0x01

//TX = Port D 3 <Debug>
//RX = Port D 2

//TX = Port C 7
//RX = Port C 6 <RFID>

//Trigger sonar A = Port C 2
//Trigger sonar B = Port C 3
//Echo    sonar A = Port C 4
//Echo    sonar B = Port C 5

//I2C
//0x10 last RFID
//0x20 sonar A in cm
//0x30 sonar B in cm

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
volatile uint8_t switchy;

void TWIC_SlaveProcessData(void);
void init_all(void);

int main(void)
{
	init_all();
	
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
		sprintf(str, "still alive! %d\n\r", x);
		uart_puts(&uartD0, str);
		sprintf(str, "Sonar A = %4d cm\n\r", transmitArray[0x20]);
		uart_puts(&uartD0, str);
		sprintf(str, "Sonar B = %4d cm\n\r", transmitArray[0x30]);
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
}

ISR(PORTC_INT0_vect)//start uart delay
{
	TCC1.CTRLA     = TC_CLKSEL_DIV1024_gc;
}

ISR(TCC1_OVF_vect)//uart delay
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
	transmitArray[5] = 'B'; //beep -> NXT
}

ISR(TCD0_CCA_vect) //sonar A
{
	uint16_t time = TCD0.CCA;
	uint8_t cm = time/116;
	transmitArray[0x20] = cm;
	TCD0.CTRLFSET = TC_CMD_RESTART_gc;
}

ISR(TCD1_CCA_vect) //Sonar B
{
	uint16_t time = TCD1.CCA;
	uint8_t cm = time/116;
	transmitArray[0x30] = cm;
	TCD1.CTRLFSET = TC_CMD_RESTART_gc;
}

ISR(TCE0_OVF_vect) //trigger sonar, cascading
{
	if (switchy)
	{
		PORTC.OUTSET = PIN2_bm;
		_delay_us(10);
		PORTC.OUTCLR = PIN2_bm;
		switchy = 0;
	} 
	else
	{
		PORTC.OUTSET = PIN3_bm;
		_delay_us(10);
		PORTC.OUTCLR = PIN3_bm;
		switchy = 1;
	}
}

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

void init_all(void)
{
	// set port direction //
	PORTE.DIRSET = PIN0_bm; //debug led
	PORTC.DIRSET = PIN2_bm|PIN3_bm; //sonar trigger

	// set timers //
	//delay for uart read need other solution
	PORTC.INT0MASK = PIN6_bm;
	PORTC.PIN6CTRL = PORT_ISC_RISING_gc;
	PORTC.INTCTRL  = PORT_INT0LVL_LO_gc;
	TCC1.CTRLB     = TC_WGMODE_NORMAL_gc;
	TCC1.CTRLA     = TC_CLKSEL_OFF_gc;
	TCC1.INTCTRLA  = TC_OVFINTLVL_LO_gc;
	TCC1.PER       = 400;
	//sonar request
	TCE0.CTRLB     = TC_WGMODE_NORMAL_gc;
	TCE0.CTRLA     = TC_CLKSEL_DIV1024_gc;
	TCE0.INTCTRLA  = TC_OVFINTLVL_LO_gc;
	TCE0.PER       = 195;//~10Hz so 5Hz each
	//timer for sonar A
	PORTC.PIN4CTRL = PORT_ISC_BOTHEDGES_gc;
	EVSYS.CH0MUX = EVSYS_CHMUX_PORTC_PIN4_gc;
	TCD0.CTRLD = TC_EVACT_PW_gc | TC_EVSEL_CH0_gc;
	TCD0.CTRLB = TC0_CCAEN_bm | TC_WGMODE_NORMAL_gc;
	TCD0.CTRLA = TC_CLKSEL_DIV1_gc;
	TCD0.INTCTRLB = TC_CCAINTLVL_LO_gc;
	TCD0.PER = 0xFFFF;
	//timer for sonar B
	PORTC.PIN5CTRL = PORT_ISC_BOTHEDGES_gc;
	EVSYS.CH1MUX = EVSYS_CHMUX_PORTC_PIN5_gc;
	TCD1.CTRLD = TC_EVACT_PW_gc | TC_EVSEL_CH1_gc;
	TCD1.CTRLB = TC1_CCAEN_bm | TC_WGMODE_NORMAL_gc;
	TCD1.CTRLA = TC_CLKSEL_DIV1_gc;
	TCD1.INTCTRLB = TC_CCAINTLVL_LO_gc;
	TCD1.PER = 0xFFFF;
	
	// set uart's //
	init_uart(&uartD0, &USARTD0, F_CPU, D0_BAUD, D0_CLK2X); //debug
	init_uart(&uartC1, &USARTC1, F_CPU, C1_BAUD, C1_CLK2X); //RFID reader
	
	// set TWI as slave for NXT //
	TWI_SlaveInitializeDriver(&twiSlave, &TWIC, TWIC_SlaveProcessData);
	TWI_SlaveInitializeModule(&twiSlave, SLAVE_ADDRESS, TWI_SLAVE_INTLVL_MED_gc);
	
	// Turnon interrupts //
	PMIC.CTRL = PMIC_LOLVLEN_bm|PMIC_MEDLVLEN_bm;
	sei();
}