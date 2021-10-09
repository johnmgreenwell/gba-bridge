/*------------------------------------------------------------------------/
/-------------------------- AVR SPI Functions ----------------------------/
/-------------------------------------------------------------------------/
/	AUTHOR
/		John Greenwell
/
/	VERSIONS
/		1.3 -- 05 July 2013
/		1.2 -- 15 April 2013
/		1.1 -- 06 March 2013 
/		1.0 -- 11 January 2013
/-------------------------------------------------------------------------/
/	DESCRIPTION
/		This include file provides access to simple Atmel AVR SPI operations
/		which can be combined to perform more meaningful functions.
/
/		Before any other of the functions defined below are used, the
/		function spi_init() should be called.
/-------------------------------------------------------------------------/
/	NOTES
/		Remember to update F_CPU in spi.c, as well as the port and pin
/		assignments below.
/
/		The CS pin is meant to be controlled externally to this file for
/		flexibility.
/
/		Note that the SS pin on, for example, the ATmega328, needs to be
/		held high or else the device mistakenly translates slave mode SPI.
/		Leaving the SS pin as a floating input can cause this to happen.
/-------------------------------------------------------------------------/
/------------------------------------------------------------------------*/

#ifndef SPI_H_
#define SPI_H_

/*--------------------------------------------------------------------------*/
/* SPI Frequency Assignments -- See Datasheet for SPCR and SPSR             */
/*--------------------------------------------------------------------------*/
#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny85__)
	// Placeholder for ATtiny
#else
 #define SPI_PRES	1	/* [0]:fsc/4,[1]:fsc/16,[2]:fsc/64,[3]fsc/128 */
 #define SPI_2X		1	/* 0: SPI2X 0 or 1: SPI2X 1 */
#endif

/*--------------------------------------------------------------------------*/
/* SPI Port and Pin Assignments                                             */
/*--------------------------------------------------------------------------*/
#define SPI_PORT	PORTB
#define SPI_DDR		DDRB
#define SPI_MOSI	2
#define SPI_MISO	3
#define SPI_SCK		1

/*--------------------------------------------------------------------------*/
/* SPI Functions                                                            */
/*--------------------------------------------------------------------------*/
extern void spi_init();
extern unsigned char spi_rw(unsigned char data);


#endif /* SPI_H_ */