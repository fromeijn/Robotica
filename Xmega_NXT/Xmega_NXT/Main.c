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
#define RFID_UART_NUM_BYTES 16

//TWI
#define SLAVE_ADDRESS 0x01

#define RFID_DETECT_ADDRESS 0x00
#define RFID_NUMBER_ADDRESS 0x01
#define SONAR_A_ADDRESS 0x0B
#define SONAR_B_ADDRESS 0x0D
#define RFID_DETECT_RESET_ADDRES 0x10

#define LINE_ADDRESS_0 0x20
#define LINE_ADDRESS_1 0x22
#define LINE_ADDRESS_2 0x24
#define LINE_ADDRESS_3 0x26

#define RFID_DETECT_BYTES 1
#define RFID_NUMBER_BYTES 10
#define SONAR_A_BYTES 2
#define SONAR_B_BYTES 2

#include "uart.h"
#include "twi_slave_driver.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

TWI_Slave_t twiSlave;

char str[256]; //uart debug temp
uint8_t TWIOut[0xFF]; //TWI out array
volatile uint8_t sonarSwitch; //for sonar trigger

void TWIC_SlaveProcessData(void);
void init_all(void);

int main(void)
{
	init_all();
	TWIOut[RFID_NUMBER_ADDRESS] = 'N';
	TWIOut[RFID_NUMBER_ADDRESS+1] = 'o';
	TWIOut[RFID_NUMBER_ADDRESS+2] = ' ';
	TWIOut[RFID_NUMBER_ADDRESS+3] = 'D';
	TWIOut[RFID_NUMBER_ADDRESS+4] = 'a';
	TWIOut[RFID_NUMBER_ADDRESS+5] = 't';
	TWIOut[RFID_NUMBER_ADDRESS+6] = 'a';
	sprintf(str, "UART Connected!!!\n\r");
	uart_puts(&uartD0, str);
	
	while(1)
	{
		sprintf(str, "Sonar A = %d cm\n\r", ((TWIOut[SONAR_A_ADDRESS+1]<<8)+TWIOut[SONAR_A_ADDRESS]));
		uart_puts(&uartD0, str);
		sprintf(str, "Sonar B = %d cm\n\r", ((TWIOut[SONAR_B_ADDRESS+1]<<8)+TWIOut[SONAR_B_ADDRESS]));
		uart_puts(&uartD0, str);

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

	uint8_t RFIDDataIn[RFID_UART_NUM_BYTES]; //RFID data
	for(int i=0; i<RFID_UART_NUM_BYTES; i++)
	{
		if (USART_RXBufferData_Available(&uartC1))
		{
			RFIDDataIn[i] = USART_RXBuffer_GetByte(&uartC1);
		}else{
			RFIDDataIn[i] = '_';
		}
	}
	
	for(int i=0; i<RFID_NUMBER_BYTES; i++)
	{
		TWIOut[RFID_NUMBER_ADDRESS+i] = RFIDDataIn[i+1];;
	}

	TWIOut[RFID_DETECT_ADDRESS] = 1;
}

ISR(TCD0_CCA_vect) //sonar A
{
	uint16_t time = TCD0.CCA;
	uint16_t cm = time/116;
	TWIOut[SONAR_A_ADDRESS] = cm & 0x00FF;	//LSB
	TWIOut[SONAR_A_ADDRESS+1] = cm>>8;		//MSB
	TCD0.CTRLFSET = TC_CMD_RESTART_gc;
}

ISR(TCD1_CCA_vect) //Sonar B
{
	uint16_t time = TCD1.CCA;
	uint16_t cm = time/116;
	TWIOut[SONAR_B_ADDRESS] = cm & 0x00FF;	//LSB
	TWIOut[SONAR_B_ADDRESS+1] = cm>>8;		//MSB
	TCD1.CTRLFSET = TC_CMD_RESTART_gc;
}

ISR(TCE0_OVF_vect) //trigger sonar, cascading
{
	if (sonarSwitch)
	{
		PORTC.OUTSET = PIN2_bm;
		_delay_us(10);
		PORTC.OUTCLR = PIN2_bm;
		sonarSwitch = 0;
	} 
	else
	{
		PORTC.OUTSET = PIN3_bm;
		_delay_us(10);
		PORTC.OUTCLR = PIN3_bm;
		sonarSwitch = 1;
	}
}

void TWIC_SlaveProcessData(void)
{
	uint8_t askbyte = twiSlave.receivedData[0];
	for(uint8_t i = 0; i < 16; i++) //give the right information back
	{
		twiSlave.sendData[i] = TWIOut[i+askbyte];
	}
	
	if(twiSlave.receivedData[0] == RFID_DETECT_RESET_ADDRES) TWIOut[RFID_DETECT_ADDRESS] = 0; //card has been read
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
	
	// Turn on interrupts //
	PMIC.CTRL = PMIC_LOLVLEN_bm|PMIC_MEDLVLEN_bm;
	sei();
}	
