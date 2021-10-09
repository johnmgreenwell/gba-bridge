/*------------------------------------------------------------------------/
/-------------------------- AVR IOX Functions ----------------------------/
/-------------------------------------------------------------------------/
/	AUTHOR
/		John Greenwell
/
/	VERSIONS
/		1.1 -- 30 March 2014
/		1.0 -- 10 August 2013
/-------------------------------------------------------------------------/
/	DESCRIPTION
/		This include file provides functions for communicating with
/		Microchip's MCP23008 (I2C) and MCP23S08 (SPI) 8-bit IO expanders.
/
/		The functions defined below are available to write data to the
/		registers on the IO expanders using either I2C or SPI as indicated.
/
/-------------------------------------------------------------------------/
/	NOTES
/		When I2C is used, the read and write functions will return an 8-bit
/		unsigned char. This value is the exit code for I2C built-in
/		operations, and will be zero for success. Non-zero value indicates
/		an error during I2C communication. 	
/
/		The use of SPI will require 4 pins: MOSI, MISO, SCK, and CS. CS
/		cannot simply be tied low because its low pulse indicates one
/		command window to the chip. MOSI, MISO, and SCK port and pin
/		definitions should be handled in the spi library brought in
/		by spi.c. CS is not handled in this library and must be handled
/		by external code (i.e. CS_low() -> iox_read/write() -> CS_high())
/		This allows for multiple MCP23S08 chips to be used.
/
/-------------------------------------------------------------------------/
/------------------------------------------------------------------------*/

#ifndef IOX_H_
#define IOX_H_


/*-------------------------------------------------------------------------*/
/* IOX Operational Options                                                 */
/*-------------------------------------------------------------------------*/
#define IOX_MODE	1		/* 0: Use I2C, 1: Use SPI */

#define IOX_OPCODE	0x40	/* Device Control Code (hardware dependent) */


/*-------------------------------------------------------------------------*/
/* IOX Memory Map                                                          */
/*-------------------------------------------------------------------------*/
/* IODIR - IO Direction Register */
#define IOX_IODIR	0x00
/* IPOL - Input Polarity Port Register */
#define IOX_IPOL	0x01
/* GPINTEN - Interrupt-on-Change Pins */
#define IOX_GPINTEN	0x02
/* DEFVAL - Default Value Register */
#define IOX_DEFVAL	0x03
/* INTCON - Interrupt-on-Change Control Register */
#define IOX_INTCON	0x04
/* IOCON - IO Expander Configuration Register */
#define IOX_IOCON	0x05
 #define IOX_SEQOP	5	/* Sequential Operation Mode Bit */
 #define IOX_DISSLW	4	/* Slew Rate Control Bit for SDA */
 #define IOX_HAEN	3	/* Hardware Address Enable Bit */
 #define IOX_ODR	2	/* Open-Drain Output on INT Pin */
 #define IOX_INTPOL	1	/* Polarity for Output on INT Pin */
/* GPPU - GPIO Pull-Up Resister Register */
#define IOX_GPPU	0x06
/* INTF - Interrupt Flag Register */
#define IOX_INTF	0x07
/* INTCAP - Interrupt Captured Value for Port Register */
#define IOX_INTCAP	0x08
/* GPIO - General Purpose IO Port Register */
#define IOX_GPIO	0x09
/* OLAT - Output Latch Register */
#define IOX_OLAT	0x0A


/*-------------------------------------------------------------------------*/
/* IOX Functions                                                           */
/*-------------------------------------------------------------------------*/
void iox_init();
#if IOX_MODE==1
 void iox_write(uint8_t addr, uint8_t data);
 void iox_read(uint8_t addr, uint8_t * data_ptr);
#else
 uint8_t iox_write(uint8_t addr, uint8_t data);
 uint8_t iox_read(uint8_t addr, uint8_t * data_ptr);
#endif
#define iox_write_gpio(data)	iox_write(IOX_GPIO,data)
#define iox_read_gpio(data_ptr)	iox_read(IOX_GPIO,data_ptr)


#endif /* IOX_H_ */