/*
 * spi.h
 *
 * Created: 2/17/2014 2:42:22 PM
 *  Author: Sander
 */ 


#ifndef SPI_H_
#define SPI_H_

#define SPI_LATCH_bm PIN0_bm
#define SPI_SS_bm PIN4_bm
#define SPI_MOSI_bm PIN5_bm
#define SPI_MISO_bm PIN6_bm
#define SPI_SCK_bm PIN7_bm
#define FOO 0

void spi_init(void);
uint8_t spi_transfer(uint8_t);
void latch_enable(void);



#endif /* SPI_H_ */