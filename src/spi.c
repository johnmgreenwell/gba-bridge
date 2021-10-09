/*-------------------------------------------------------------/
/
/	FILE: spi.c
/
/	VERSION: 1.3 -- 05 July 2013
/
/	AUTHOR: John Greenwell
/
/	DESCRIPTION:
/	This c file contains functions referenced in spi.h for
/	Atmel AVR SPI communication.
/
/	NOTES:
/	See spi.h for more information.
/
/-------------------------------------------------------------*/
#include "spi.h"

#include <avr/io.h>
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny85__)
 #include <avr/interrupt.h>
#endif

#if SPI_PRES > 3
#error "SPI_PRES cannot exceed 3"
#endif

// Initializes SPI master mode
void spi_init() {
	// MOSI, SCK as outputs, MISO as input, (Init CS Separately)
	SPI_DDR |= (1<<SPI_MOSI)|(1<<SPI_SCK);
	SPI_DDR &= ~(1<<SPI_MISO);
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny85__)
	// USI SPI, Master Mode
	USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK);
#else
	// Enable SPI, Master
	SPCR = (1<<SPE)|(1<<MSTR)|(SPI_PRES);
	SPSR = (1<<SPI_2X);
#endif
}

unsigned char spi_rw(unsigned char data) {
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny85__)
	cli();
	USIDR = data;
	USISR = (1<<USIOIF);
	while (!(USISR & (1<<USIOIF))) USICR |= (1<<USITC);		 
	return USIDR;
	sei();
#else
	// Start transmission
	SPDR = data;
	// Wait for flag transmission complete
	while(!(SPSR & (1<<SPIF)));
	// Return data now contained in SPDR
	return SPDR;
#endif
}