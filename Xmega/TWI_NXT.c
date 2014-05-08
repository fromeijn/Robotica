/*
 * TWI_NXT.c
 *
 * Created: 7-5-2014 12:10:23
 *  Author: Floris
 */ 

#define F_CPU 2000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "avr_compiler.h"
#include "twi_slave_driver.h"

#define SLAVE_ADDRESS 0x01
TWI_Slave_t twiSlave;

char indata[];

void TWIC_SlaveProcessData(void)
{
	uint8_t index = twiSlave.bytesReceived;
	indata[index] = twiSlave.receivedData[index];
}

int main(void)
{
	PORTE.DIRSET = PIN0_bm;
	PORTE.OUTSET = PIN0_bm;
	TWI_SlaveInitializeDriver(&twiSlave, &TWIC, TWIC_SlaveProcessData);
	TWI_SlaveInitializeModule(&twiSlave, SLAVE_ADDRESS, TWI_SLAVE_INTLVL_LO_gc);

	PMIC.CTRL |= PMIC_LOLVLEN_bm;
	sei();
	
	twiSlave.sendData[0] = 'N';
	twiSlave.sendData[1] = 'u';
	twiSlave.sendData[2] = 'm';
	twiSlave.sendData[3] = 'm';
	twiSlave.sendData[4] = 'e';
	twiSlave.sendData[5] = 'r';
	twiSlave.sendData[6] = ' ';
	
	while (1) {
		for (int x = 0; x<10; x++)
		{
			twiSlave.sendData[7] = '0'+x;
			_delay_ms(1000);
		}
	}
}


ISR(TWIC_TWIS_vect)
{
	TWI_SlaveInterruptHandler(&twiSlave);
	PORTE.OUTTGL = PIN0_bm;
}