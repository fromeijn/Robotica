/*
 * Controller.c
 *
 *	RFID	Xmega
 *	3.3V	3.3V
 *	RST		C0
 *	GND		GND
 *	RO		NC
 *	MISO	C6
 *	MOSI	C5
 *	SCK		C7
 *	SDA		C4
 *
 * Created: 6-5-2014 19:46:14
 *  Author: Dedier
 */ 


#define F_CPU     2000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define ENABLE_UART_D0  1
#define D0_BAUD			115200
#define D0_CLK2X		0

#include "uart.h"
#include "MFRC552.h"
#include "spi.h"


/*uchar serNum[5];

uchar  writeData[16]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100};  //??? 100??
uchar  moneyConsume = 18 ;
uchar  moneyAdd = 10 ;

uchar sectorKeyA[16][16] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
//{0x19, 0x84, 0x07, 0x15, 0x76, 0x14},
{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
};
uchar sectorNewKeyA[16][16] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x84,0x07,0x15,0x76,0x14},
//you can set another ket , such as  " 0x19, 0x84, 0x07, 0x15, 0x76, 0x14 "
//{0x19, 0x84, 0x07, 0x15, 0x76, 0x14, 0xff,0x07,0x80,0x69, 0x19,0x84,0x07,0x15,0x76,0x14},
// but when loop, please set the  sectorKeyA, the same key, so that RFID module can read the card
{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x33,0x07,0x15,0x34,0x14},
};

*/



int main(void)
{
	PORTE.DIRSET = PIN0_bm;
	
	init_uart(&uartD0, &USARTD0, F_CPU, D0_BAUD, D0_CLK2X);
	PMIC.CTRL = PMIC_LOLVLEN_bm;
	sei();
	
	spi_init();
	
	
	
	PORTC.DIRSET = chipSelectPin;
	PORTC.OUTCLR = chipSelectPin;
	PORTC.DIRSET = NRST;
	PORTC.OUTSET = NRST;
	
	//MFRC522_Init();
		
	uart_puts(&uartD0, "init succeeded\n");
	_delay_ms(1000);
	
	char data[100];
	uint8_t info = Read_MFRC522(ErrorReg);
	sprintf(data, " %d \n", info);
	uart_puts(&uartD0, data);
	
    while(1)
    {
		/*uchar temp,i;
		uchar status;
		uchar str[MAX_LEN];
		uchar RC_size;
		uchar blockAddr;
		char mynum[] = "";
		
		
		status = MFRC522_Anticoll(str);
		memcpy(serNum, str, 5);
		if (status == MI_OK)
		{
			uart_puts(&uartD0, "The card's number is  : ");
			uart_puts(&uartD0, serNum[0]);
		}*/
        PORTE.OUTTGL = PIN0_bm;
		uart_puts(&uartD0, "hallo\n");
		_delay_ms(1000);
    }
	
}