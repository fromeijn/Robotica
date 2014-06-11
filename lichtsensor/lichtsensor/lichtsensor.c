/*
 * lichtsensor.c
 *
 * Created: 10-6-2014 10:16:57
 *  Author: Lang Griffith
 */ 

#define F_CPU          2000000UL
#define ENABLE_UART_D0 1
#define D0_BAUD        115200
#define D0_CLK2X       0
#include <avr/io.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "uart.h"
#define ADC_CH_MUXNEG_GND_gc 0x05 // see table 28-16 au-manual
#define ADC_CH_MUXNEG_INTGND_gc 0x07 // see table 28-16 au-manual

  int res;
  int sensor1 = PIN0_bm;
  int sensor2 = PIN1_bm;
  int sensor3 = PIN2_bm;
  int sensor4 = PIN3_bm;
  int line;
  int line1;
 
   void set_adcch_input(ADC_CH_t *ch, uint8_t pos_pin_gc, uint8_t neg_pin_gc)
   {
	   ch->MUXCTRL = pos_pin_gc | neg_pin_gc;
	   ch->CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
   }
   void init_adc(void)
   {
	   PORTA.DIRCLR = PIN3_bm|PIN2_bm|PIN1_bm|PIN0_bm; // PA3..0 are input
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
   
int main(void)
{
	char str[256];
	init_adc();
	init_uart(&uartD0, &USARTD0, F_CPU, D0_BAUD, D0_CLK2X);
	PMIC.CTRL = PMIC_LOLVLEN_bm;
	sei();
	
	while(1)
	{
		uint16_t ch0_value = ADCA.CH0.RES;	
		sprintf(str, "%d\r\n", ch0_value);
		uart_puts(&uartD0, str);
		_delay_ms(50);
	}
}
	
