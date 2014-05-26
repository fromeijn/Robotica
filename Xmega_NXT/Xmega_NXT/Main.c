/* PORTS 

Debug
TX 		= Port D 3
RX 		= Port D 2

RFID
TX 		= Port C 7
RX 		= Port C 6

Sonar A
Trigger = Port C 2
Echo 	= Port C 4

Sonar B
Trigger = Port C 3
Echo   	= Port C 5

I2C Adresses
0x00 card detection and still running check
0x01 card has been read
0x10 last RFID
0x20 sonar A in cm, 0x20 LSB, 0x21 MSB 
0x30 sonar B in cm, 0x30 LSB, 0x31 MSB 
*/

#define F_CPU     2000000UL

//Debug
#define ENABLE_UART_D0  1
#define D0_BAUD			115200
#define D0_CLK2X		0

//RFID
#define ENABLE_UART_C1  1
#define C1_BAUD			9600
#define C1_CLK2X		0
#define NUM_BYTES 16

//TWI
#define SLAVE_ADDRESS 0x01

#include "uart.h"
#include "twi_slave_driver.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

TWI_Slave_t twiSlave;

char str[256]; //uart debug temp
uint8_t RFIDin[NUM_BYTES]; //RFID data
uint8_t TWIout[256]; //TWI out array
volatile uint8_t sonarswitch; //for sonar trigger

void TWIC_SlaveProcessData(void);
void init_all(void);

int main(void)
{
	init_all();
	
	sprintf(str, "UART Connected!!!\n\r");
	uart_puts(&uartD0, str);
	
	TWIout[0x00] = 'T';
	TWIout[0x01] = 'W';
	TWIout[0x02] = 'I';
	TWIout[0x03] = ' ';
	
	uint8_t x = 0;
	while(1)
	{
		TWIout[4] = '0'+x;
		sprintf(str, "Sonar A = %d cm\n\r", ((TWIout[0x21]<<8)+TWIout[0x20]));
		uart_puts(&uartD0, str);
		sprintf(str, "Sonar B = %d cm\n\r", ((TWIout[0x31]<<8)+TWIout[0x30]));
		uart_puts(&uartD0, str);
		x++;
		if (x>=10)
		{
			x = 0;
		}
		_delay_ms(1000);
	}
}

ISR(TWIC_TWIS_vect) //TWI
{
	TWI_SlaveInterruptHandler(&twiSlave);
}

ISR(PORTC_INT0_vect) //start uart delay
{
	TCC1.CTRLA     = TC_CLKSEL_DIV1024_gc;
}

ISR(TCC1_OVF_vect) //uart delay
{
	TCC1.CTRLA     = TC_CLKSEL_OFF_gc;
	int i = 0;
	while (i < NUM_BYTES)
	{
		if (USART_RXBufferData_Available(&uartC1))
		{
			RFIDin[i] = USART_RXBuffer_GetByte(&uartC1);
		}else{
			RFIDin[i] = '_';
		}
		i++;
	}
	
	i = 0;
	uart_puts(&uartD0, "ID: ");
	while (i < NUM_BYTES)
	{
		TWIout[i+0x0F] = RFIDin[i];
		uart_putc(&uartD0, RFIDin[i]);
		i++;
	}
	TWIout[5] = 'B'; //beep -> NXT
}

ISR(TCD0_CCA_vect) //sonar A
{
	uint16_t time = TCD0.CCA;
	uint16_t cm = time/116;
	TWIout[0x20] = cm & 0x00FF;	//LSB
	TWIout[0x21] = cm>>8;		//MSB
	TCD0.CTRLFSET = TC_CMD_RESTART_gc;
}

ISR(TCD1_CCA_vect) //Sonar B
{
	uint16_t time = TCD1.CCA;
	uint16_t cm = time/116;
	TWIout[0x30] = cm & 0x00FF;	//LSB
	TWIout[0x31] = cm>>8;		//MSB
	TCD1.CTRLFSET = TC_CMD_RESTART_gc;
}

ISR(TCE0_OVF_vect) //trigger sonar, cascading
{
	if (sonarswitch)
	{
		PORTC.OUTSET = PIN2_bm;
		_delay_us(10);
		PORTC.OUTCLR = PIN2_bm;
		sonarswitch = 0;
	} 
	else
	{
		PORTC.OUTSET = PIN3_bm;
		_delay_us(10);
		PORTC.OUTCLR = PIN3_bm;
		sonarswitch = 1;
	}
}

void TWIC_SlaveProcessData(void)
{
	uint8_t askbyte = twiSlave.receivedData[0];
	//give the right information back
	for(uint8_t i = 0; i < 16; i++)
	{
		twiSlave.sendData[i] = TWIout[i+askbyte];
	}
	if(twiSlave.receivedData[0] == 0x01) TWIout[5] = ' '; //card has been read
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
