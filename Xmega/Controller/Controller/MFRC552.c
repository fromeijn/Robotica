/*
 * MFRC552.c
 *
 * Created: 6-5-2014 21:16:46
 *  Author: Dedier
 */ 

#include <avr/io.h>
#include "MFRC552.h"
#include "spi.h"

//uint8_t spi_transfer(uint8_t data);

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
void Write_MFRC522(uchar addr, uchar val)
{
	uchar address = (addr<<1) & 0x7E;
	PORTC.OUTCLR = chipSelectPin;
	
	PORTE.OUTSET = PIN0_bm;
	spi_transfer(address);
	spi_transfer(val);
	
	PORTC.OUTSET = chipSelectPin;
}

uchar Read_MFRC522(uchar addr)
{
	uchar val;
	uchar addres = ((addr<<1) & 0x7E | 0x80);
	
	PORTC.OUTCLR = chipSelectPin;
	
	spi_transfer(addres);
	val = spi_transfer(0x00);
	
	PORTC.OUTSET = chipSelectPin;
	
	return val;
}

void SetBitMask(uchar reg, uchar mask)
{
	uchar tmp;
	tmp = Read_MFRC522(reg);
	Write_MFRC522(reg, tmp | mask);
}

void ClearBitMask(uchar reg, uchar mask)
{
	uchar tmp;
	tmp = Read_MFRC522(reg);
	Write_MFRC522(reg, tmp & (~mask));
}

void AntennaOn(void)
{
	uchar tmp;
	
	tmp = Read_MFRC522(TxControlReg);
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
	PORTE.OUTSET = PIN0_bm;
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


uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen)
{
	uchar status = MI_ERR;
	uchar irqEn = 0x00;
	uchar waitIRq = 0x00;
	uchar lastBits;
	uchar n;
	uint i;

	switch (command)
	{
		case PCD_AUTHENT:		//????
		{
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE:	//??FIFO???
		{
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
		break;
	}
	
	Write_MFRC522(CommIEnReg, irqEn|0x80);	//??????
	ClearBitMask(CommIrqReg, 0x80);			//?????????
	SetBitMask(FIFOLevelReg, 0x80);			//FlushBuffer=1, FIFO???
	
	Write_MFRC522(CommandReg, PCD_IDLE);	//NO action;??????	???

	//?FIFO?????
	for (i=0; i<sendLen; i++)
	{
		Write_MFRC522(FIFODataReg, sendData[i]);
	}

	//????
	Write_MFRC522(CommandReg, command);
	if (command == PCD_TRANSCEIVE)
	{
		SetBitMask(BitFramingReg, 0x80);		//StartSend=1,transmission of data starts
	}
	
	//????????
	i = 2000;	//i???????????M1???????25ms	???
	do
	{
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = Read_MFRC522(CommIrqReg);
		i--;
	}
	while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	ClearBitMask(BitFramingReg, 0x80);			//StartSend=0
	
	if (i != 0)
	{
		if(!( Read_MFRC522(ErrorReg) & 0x1B))	//BufferOvfl Collerr CRCErr ProtecolErr
		{
			status = MI_OK;
			if (n & irqEn & 0x01)
			{
				status = MI_NOTAGERR;			//??
			}

			if (command == PCD_TRANSCEIVE)
			{
				n = Read_MFRC522(FIFOLevelReg);
				lastBits = Read_MFRC522(ControlReg) & 0x07;
				if (lastBits)
				{
					*backLen = (n-1)*8 + lastBits;
				}
				else
				{
					*backLen = n*8;
				}

				if (n == 0)
				{
					n = 1;
				}
				if (n > MAX_LEN)
				{
					n = MAX_LEN;
				}
				
				//??FIFO???????
				for (i=0; i<n; i++)
				{
					backData[i] = Read_MFRC522(FIFODataReg);
				}
			}
		}
		else
		{
			status = MI_ERR;
		}
		
	}
	
	//SetBitMask(ControlReg,0x80);           //timer stops
	//Write_MFRC522(CommandReg, PCD_IDLE);

	return status;
}


uchar MFRC522_Anticoll(uchar *serNum)
{
	uchar status;
	uchar i;
	uchar serNumCheck=0;
	uint unLen;
	

	//ClearBitMask(Status2Reg, 0x08);		//TempSensclear
	//ClearBitMask(CollReg,0x80);			//ValuesAfterColl
	Write_MFRC522(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
	
	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK)
	{
		//??????
		for (i=0; i<4; i++)
		{
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i])
		{
			status = MI_ERR;
		}
	}

	//SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1

	return status;
}


