#include <avr/io.h>
#include "spi.h"

void spi_init(void) {
	PORTC.DIRSET  =  SPI_SCK_bm|SPI_MOSI_bm|SPI_SS_bm|SPI_LATCH_bm;
	PORTC.DIRCLR  =  SPI_MISO_bm;
	PORTC.OUTCLR  =  SPI_SS_bm;
	SPIC.CTRL =  SPI_ENABLE_bm | SPI_MASTER_bm | SPI_CLK2X_bm /*| (SPI_DORD_bm)*/  | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc;
}

uint8_t spi_transfer(uint8_t data)
{
	SPIC.DATA = data;
	while(!(SPIC.STATUS & (SPI_IF_bm)));
	return SPIC.DATA; 
}

inline void latch_enable(void)
{
	PORTC.OUTSET = SPI_LATCH_bm;
	PORTC.OUTCLR = SPI_LATCH_bm;
}