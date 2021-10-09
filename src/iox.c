/*-------------------------------------------------------------/
/
/	FILE: iox.c
/
/	VERSION: 1.1 -- 30 March 2014
/
/	AUTHOR: John Greenwell
/
/	DESCRIPTION:
/	This c file contains functions for IO port expansion with
/	Microchip's MCP23008 (I2C) and MCP23S08 (SPI) 8-bit IO
/	expanders.
/
/	NOTES:
/	See iox.h for more information.
/
/-------------------------------------------------------------*/
#include <avr/io.h>
#include <stdint.h>
#include "iox.h"
#if IOX_MODE==1
 #include "spi.h"
#else
 #include "i2c.h"
#endif


/*---------------------------------------------------------*/
/* IOX Initialization                                      */
/*---------------------------------------------------------*/
void iox_init() {
#if IOX_MODE==1
	spi_init();
#else
	i2c_init();
#endif	
} 


/*---------------------------------------------------------*/
/* IOX Write Byte -- SPI                                   */
/*---------------------------------------------------------*/
#if IOX_MODE==1
void iox_write(uint8_t addr, uint8_t data) {
	spi_rw(IOX_OPCODE|0);
	spi_rw(addr);
	spi_rw(data);
}


/*---------------------------------------------------------*/
/* IOX Write Byte -- I2C                                   */
/*---------------------------------------------------------*/
#else
uint8_t iox_write(uint8_t addr, uint8_t data) {
	uint8_t res = 0;
	res = i2c_start();
	if (res) { return res; }
	i2c_write(IOX_OPCODE|I2C_WRITE);
	i2c_write(addr);
	res = i2c_write(data);
	if (res) { return res; }
	i2c_stop();
	return 0;
}
#endif


/*---------------------------------------------------------*/
/* IOX Read Byte -- SPI                                    */
/*---------------------------------------------------------*/
#if IOX_MODE==1
void iox_read(uint8_t addr, uint8_t * data_ptr) {
	spi_rw(IOX_OPCODE|1);
	spi_rw(addr);
	*data_ptr = spi_rw(0xFF);	// Dummy byte send
}


/*---------------------------------------------------------*/
/* IOX Read Byte -- I2C                                    */
/*---------------------------------------------------------*/
#else
uint8_t iox_read(uint8_t addr, uint8_t * data_ptr) {
	uint8_t res = 0;
	res = i2c_start();
	if (res) { return res; }
	i2c_write(IOX_OPCODE|I2C_WRITE);
	i2c_write(addr);
	i2c_start(); // Repeated start
	i2c_write(IOX_OPCODE|I2C_READ);
	res = i2c_read(data_ptr,I2C_NACK);
	if (res) { return res; }
	i2c_stop();
	return 0;
}
#endif