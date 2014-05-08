/*
 * MFRC552.c
 *
 * Created: 6-5-2014 21:16:46
 *  Author: Dedier
 */ 

#include <avr/io.h>
#include "MFRC552.h"

uint8_t spi_transfer(uint8_t data);

uchar serNum[5];

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

void Write_MFRC522(uchar addr, uchar val)
{
	PORTC.OUTCLR = chipSelectPin;
	
	spi_transfer((addr<<1) &0x7E);
	spi_transfer(val);
	
	PORTC.OUTSET = chipSelectPin;
}

uchar Read_MFRC552(uchar addr)
{
	uchar val;
	
	PORTC.OUTCLR = chipSelectPin;
	
	spi_transfer(((addr<<1) &0x7E) | 0x80);
	val = spi_transfer(0x00);
	
	PORTC.OUTSET = chipSelectPin;
	
	return val;
}

void SetBitMask(uchar reg, uchar mask)
{
	uchar tmp;
	tmp = Read_MFRC552(reg);
	Write_MFRC522(reg, tmp | mask);
}

void ClearBitMask(uchar reg, uchar mask)
{
	uchar tmp;
	tmp = Read_MFRC552(reg);
	Write_MFRC522(reg, tmp & (~mask));
}

void AntennaOn(void)
{
	uchar tmp;
	
	tmp = Read_MFRC552(TxControlReg);
	if (!(tmp & 0x03))
	{
		SetBitMask(TxControlReg, 0x03);
	}
}

void AntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}

void MFRC522_Reset(void)
{
	Write_MFRC522(CommandReg, PCD_RESETPHASE);
}


void MFRC522_Init(void)
{
	PORTC.OUTSET = NRST;
	
	MFRC522_Reset();
	
	Write_MFRC522(TModeReg, 0x8D);
	Write_MFRC522(TPrescalerReg, 0x3E);	//TModeReg[3..0] + TPrescalerReg
	Write_MFRC522(TReloadRegL, 30);
	Write_MFRC522(TReloadRegH, 0);
	
	Write_MFRC522(TxAutoReg, 0x40);		//100%ASK
	Write_MFRC522(ModeReg, 0x3D);		//CRC 0x6363	???

	//ClearBitMask(Status2Reg, 0x08);		//MFCrypto1On=0
	//Write_MFRC522(RxSelReg, 0x86);		//RxWait = RxSelReg[5..0]
	//Write_MFRC522(RFCfgReg, 0x7F);   		//RxGain = 48dB

	AntennaOn();
	
}