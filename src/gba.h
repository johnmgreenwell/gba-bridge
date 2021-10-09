/*------------------------------------------------------------------------/
/-------------------------- AVR GBA Functions ----------------------------/
/-------------------------------------------------------------------------/
/	AUTHOR
/		John Greenwell
/
/	VERSIONS
/		1.0 -- 31 March 2014
/-------------------------------------------------------------------------/
/	DESCRIPTION
/		This include file provides access to simple Atmel AVR GBA
/		functions.
/-------------------------------------------------------------------------/
/	NOTES
/		
/-------------------------------------------------------------------------/
/------------------------------------------------------------------------*/
#ifndef GBA_H_
#define GBA_H_

#ifndef F_CPU
 #define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"
#include "ff.h"
#include "lcd.h"


/* PORT, DDR, and PIN Definitions */
#define RD_PORT		PORTB
#define RD_DDR		DDRB
#define RD_PIN		4
#define WR_PORT		PORTB
#define WR_DDR		DDRB
#define WR_PIN		5
#define CS_PORT		PORTB
#define CS_DDR		DDRB
#define CS_PIN		6
#define CS2_PORT	PORTB
#define CS2_DDR		DDRB
#define CS2_PIN		7
#define HIGH_PORT	PORTD
#define HIGH_DDR	DDRD
#define HIGH_PIN	PIND
#define MID_PORT	PORTC
#define MID_DDR		DDRC
#define MID_PIN		PINC
#define LOW_PORT	PORTA
#define LOW_DDR		DDRA
#define LOW_PIN		PINA

/* GBA LED and Buttons */
#define GBA_LED_PORT	PORTE
#define GBA_LED_DDR		DDRE
#define GBA_LED_PIN		2
#define GBA_B1_PORT		PORTE
#define GBA_B1_DDR		DDRE
#define GBA_B1_PIN		PINE
#define GBA_B1_NUM		3
#define GBA_B2_PORT		PORTE
#define GBA_B2_DDR		DDRE
#define GBA_B2_PIN		PINE
#define GBA_B2_NUM		4


/* GBA LED and Buttons Control */
#define GBA_LED_INIT()	GBA_LED_DDR |= (1<<GBA_LED_PIN);\
						GBA_LED_PORT &= ~(1<<GBA_LED_PIN)
#define GBA_CHECK_LED()	GBA_LED_PORT & (1<<GBA_LED_PIN)
#define GBA_LED_ON()	GBA_LED_PORT |= (1<<GBA_LED_PIN)
#define GBA_LED_OFF()	GBA_LED_PORT &= ~(1<<GBA_LED_PIN)
#define GBA_B1_INIT()	GBA_B1_DDR &= ~(1<<GBA_B1_NUM);\
						GBA_B1_PORT |= (1<<GBA_B1_NUM);\
						EIMSK |= (1<<PCIE0); PCMSK0 |= (1<<PCINT3)
#define GBA_B1_CHECK()	!(GBA_B1_PIN & (1<<GBA_B1_NUM))
#define GBA_B2_INIT()	GBA_B2_DDR &= ~(1<<GBA_B2_NUM);\
						GBA_B2_PORT |= (1<<GBA_B2_NUM);\
						EIMSK |= (1<<PCIE0); PCMSK0 |= (1<<PCINT4)
#define GBA_B2_CHECK()	!(GBA_B2_PIN & (1<<GBA_B2_NUM))


/* WR, RD, CS Manipulations */
#define WR_HIGH()		WR_PORT |= (1<<WR_PIN)
#define WR_LOW()		WR_PORT &= ~(1<<WR_PIN)
#define RD_HIGH()		RD_PORT |= (1<<RD_PIN)
#define RD_LOW()		RD_PORT &= ~(1<<RD_PIN)
#define CS_HIGH()		CS_PORT |= (1<<CS_PIN)
#define CS_LOW()		CS_PORT &= ~(1<<CS_PIN)
#define CS2_HIGH()		CS2_PORT |= (1<<CS2_PIN)
#define CS2_LOW()		CS2_PORT &= ~(1<<CS2_PIN)

/* EEPROM Only Single Pin Manipulations */
#define A0_OUTPUT()		DDRA |= (1<<0)
#define A0_INPUT()		DDRA &= ~(1<<0)
#define A0_HIGH()		PORTA |= (1<<0)
#define A0_LOW()		PORTA &= ~(1<<0)
#define A0_CHECK()		(PINA & (1<<0))
#define A23_OUTPUT()	DDRD |= (1<<0)
#define A23_INPUT()		DDRD &= ~(1<<0)
#define A23_HIGH()		PORTD |= (1<<0)
#define A23_LOW()		PORTD &= ~(1<<0)



/* Functions */
uint8_t get_b1_flg();
uint8_t get_b2_flg();
void reset_b1_flg();
void reset_b2_flg();
void gba_init();
void gba_write_addr(uint32_t addr);
void gba_read_addr(uint16_t * data_ptr);
uint8_t gba_get_uart();
void gba_read_rom(uint32_t size);
uint8_t gba_check_ram();
void gba_read_ram(uint8_t ram_type);
void gba_write_ram(uint8_t ram_type);
void gba_print_return_menu();

#endif // GBA_H_