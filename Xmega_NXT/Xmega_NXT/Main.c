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

#define F_CPU     32000000UL

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
#define LIGHT_REQUEST_ADDRESS 16

#define ADC_CH_MUXNEG_GND_gc 0x05 // see table 28-16 au-manual
#define ADC_CH_MUXNEG_INTGND_gc 0x07 // see table 28-16 au-manual

#define LINE_ADDRESS_0 0x20
#define LINE_ADDRESS_1 0x22
#define LINE_ADDRESS_2 0x24
#define LINE_ADDRESS_3 0x26

#define RFID_DETECT_BYTES 1
#define RFID_NUMBER_BYTES 10
#define SONAR_A_BYTES 2
#define SONAR_B_BYTES 2

#include "clksys_driver.h"
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
volatile uint16_t *lightSensor[4]; // light sensor values

void TWIC_SlaveProcessData(void);
void init_all(void);
void set_adcch_input(ADC_CH_t *, uint8_t, uint8_t);
void init_adc(void);
void clock_init32MCalibrate(void);

int main(void)
{
	clock_init32MCalibrate();
	init_all();
	TWIOut[RFID_NUMBER_ADDRESS] =	'N';
	TWIOut[RFID_NUMBER_ADDRESS+1] =	'o';
	TWIOut[RFID_NUMBER_ADDRESS+2] =	' ';
	TWIOut[RFID_NUMBER_ADDRESS+3] =	'D';
	TWIOut[RFID_NUMBER_ADDRESS+4] =	'a';
	TWIOut[RFID_NUMBER_ADDRESS+5] =	't';
	TWIOut[RFID_NUMBER_ADDRESS+6] =	'a';
	sprintf(str, "UART Connected.\n\r");
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
	if(askbyte >= 0 && askbyte < 16) { // Ask for sonar and rfid information
		for(uint8_t i = 0; i < LIGHT_REQUEST_ADDRESS; i++) //give the right information back
		{
			twiSlave.sendData[i] = TWIOut[i+askbyte];
		}
	
		if(twiSlave.receivedData[0] == RFID_DETECT_RESET_ADDRES) TWIOut[RFID_DETECT_ADDRESS] = 0; //card has been read
	} else if(askbyte == LIGHT_REQUEST_ADDRESS) { // ask for lightsensor information
		
		for(uint8_t i = 0; i < 4; i++) {
			
			twiSlave.sendData[i] = *lightSensor[i];
			
		}
		
	}
}

void set_adcch_input(ADC_CH_t *ch, uint8_t pos_pin_gc, uint8_t neg_pin_gc)
{
	ch->MUXCTRL = pos_pin_gc | neg_pin_gc;
	ch->CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
}

   void init_adc(void)
   {
	   PORTA.DIRCLR = PIN4_bm|PIN3_bm|PIN2_bm|PIN1_bm|PIN0_bm; // PA3..0 are input
	   
	   lightSensor[0] =  &ADCA.CH0.RES;
	   lightSensor[1] =  &ADCA.CH1.RES;
	   lightSensor[2] =  &ADCA.CH2.RES;
	   lightSensor[3] =  &ADCA.CH3.RES;
	   
	   set_adcch_input(&ADCA.CH0, ADC_CH_MUXPOS_PIN1_gc, ADC_CH_MUXNEG_INTGND_gc);
	   set_adcch_input(&ADCA.CH1, ADC_CH_MUXPOS_PIN2_gc, ADC_CH_MUXNEG_INTGND_gc);
	   set_adcch_input(&ADCA.CH2, ADC_CH_MUXPOS_PIN3_gc, ADC_CH_MUXNEG_INTGND_gc);
	   set_adcch_input(&ADCA.CH3, ADC_CH_MUXPOS_PIN4_gc, ADC_CH_MUXNEG_INTGND_gc);
	   ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc |
	   (!ADC_CONMODE_bm) |
	   ADC_FREERUN_bm; // free running mode
	   ADCA.REFCTRL = ADC_REFSEL_AREFA_gc;
	   ADCA.PRESCALER = ADC_PRESCALER_DIV512_gc;
	   ADCA.CTRLA = ADC_ENABLE_bm;
	   //ADCA.EVCTRL = ADC_SWEEP_0123_gc|ADC_EVSEL_0123_gc|ADC_EVACT_NONE_gc;
   }
   
void clock_init32MCalibrate(void) {
	
	// Select 32 kHz crystal and low power mode
	OSC.XOSCCTRL = ( OSC.XOSCCTRL & ~OSC_XOSCSEL_gm) | OSC_XOSCSEL_32KHz_gc;

	// Switch to calibrated 32MHz oscillator and disable 2 MHz RC oscillator
	CLKSYS_Enable( OSC_XOSCEN_bm );
	CLKSYS_Enable( OSC_RC32MEN_bm );
	do {} while ( CLKSYS_IsReady( OSC_XOSCRDY_bm ) == 0 );
	do {} while ( CLKSYS_IsReady( OSC_RC32MRDY_bm ) == 0 );
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_RC32M_gc );
	CLKSYS_Disable( OSC_RC2MEN_bm );
	OSC.DFLLCTRL = (OSC.DFLLCTRL & ~OSC_RC32MCREF_gm) | OSC_RC32MCREF_XOSC32K_gc;
	DFLLRC32M.CTRL |= DFLL_ENABLE_bm;
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
	TCC1.PER       = 400 * 16;
	//sonar request
	TCE0.CTRLB     = TC_WGMODE_NORMAL_gc;
	TCE0.CTRLA     = TC_CLKSEL_DIV1024_gc;
	TCE0.INTCTRLA  = TC_OVFINTLVL_LO_gc;
	TCE0.PER       = 195 * 16;//~10Hz so 5Hz each
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
	
	init_adc();
	
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
