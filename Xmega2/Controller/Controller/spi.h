/*
 * spi.h
 *
 * Created: 6-5-2014 8:42:22 PM
 *  Author: Dedier
 */ 


#ifndef SPI_H_
#define SPI_H_

#define SPI_MOSI_bm PIN5_bm
#define SPI_MISO_bm PIN6_bm
#define SPI_SCK_bm PIN7_bm
#define FOO 0

void spi_init(void);
uint8_t spi_transfer(uint8_t);
uint8_t spi_read_byte(void);



#endif /* SPI_H_ */