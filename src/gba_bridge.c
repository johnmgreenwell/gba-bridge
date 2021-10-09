/*---------------------------------------------------------------------------/
/  File Name    : gba_bridge.c
/  Version      : 1.0
/  Description  : GBA Cartridge Communication
/  Author(s)    : John Greenwell
/  Target(s)    : ATmega325
/  Compiler     : AVR Visual Studio, GCC 4.6.2
/  IDE          : AVR Visual Studio 6.0.1938
/  Programmer   : USBTiny Programmer
/  Last Updated : 9 May 2014
/---------------------------------------------------------------------------*/

#ifndef F_CPU
#define F_CPU	16000000
#endif

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "gba.h"


/* Sleep Function */
void sleep() {
	sleep_enable();
	sleep_cpu();
	sleep_disable();
}


/* Main Program */
int main(void) {
	
	/* Local Variables */
	uint8_t select = 0;
	uint8_t new_op = 1;
	uint8_t next_sel_b1[29] = {
		1,		// 0 Main -> 1 Read ROM select
		3,		// 1 Read ROM select -> 3 Read ROM select 2MB
		17,		// 2 Read RAM select EEPROM 8k -> 17 Execute RAM read EEPROM 8k
		10,		// 3 Read ROM select 2MB -> 10 Execute Read ROM 2MB
		18,		// 4 Read RAM select EEPROM 512 -> 18 Execute RAM read	EEPROM 512
		25,		// 5 Write RAM select okay -> 25 Write RAM select EEPROM 512
		11,		// 6 Read ROM select 4MB -> 11 Execute Read ROM 4MB
		12,		// 7 Read ROM select 8MB -> 12 Execute Read ROM 8MB
		13,		// 8 Read ROM select 16MB -> 13 Execute Read ROM 16MB
		14,		// 9 Read ROM select 32MB -> 14 Execute Read ROM 32MB
		0,		// 10 Execute Read ROM 2MB -> 0 Main
		0,		// 11 Execute Read ROM 4MB -> 0 Main
		0,		// 12 Execute Read ROM 8MB -> 0 Main
		0,		// 13 Execute Read ROM 16MB -> 0 Main
		0,		// 14 Execute Read ROM 32MB -> 0 Main
		20,		// 15 Read RAM select Flash -> 20 Execute RAM read Flash
		21,		// 16 Read RAM select F/SRAM -> 21 Execute RAM read F/SRAM
		0,		// 17 Execute RAM read EEPROM 8k -> 0 Main
		0,		// 18 Execute RAM read	EEPROM 512 -> 0 Main
		0,		// 19 Execute RAM write EEPROM 512 -> 0 Main
		0,		// 20 Execute RAM read Flash -> 0 Main
		0,		// 21 Execute RAM read F/SRAM -> 0 Main
		0,		// 22 Execute RAM write EEPROM 8k -> 0 Main
		0,		// 23 Execute RAM write Flash -> 0 Main
		0,		// 24 Execute RAM write F/SRAM -> 0 Main
		19,		// 25 Write RAM select EEPROM 512 -> 19 Execute RAM write EEPROM 512
		22,		// 26 Write RAM select EEPROM 8k -> 22 Execute RAM write EEPROM 8k
		23,		// 27 Write RAM select Flash -> 23 Execute RAM write Flash
		24};	// 28 Write RAM select F/SRAM -> 24 Execute RAM write F/SRAM
		
	uint8_t next_sel_b2[29] = {
		5,		// 0 Main -> 5 Write RAM select Flash
		4,		// 1 Read ROM select -> 4 Read RAM select Flash
		15,		// 2 Read RAM select EEPROM 8k -> 15 Read RAM select Flash
		6,		// 3 Read ROM select 2MB -> 6 Read ROM select 4MB
		2,		// 4 Read RAM select EEPROM 512 -> 2 Read RAM select EEPROM 8k
		0,		// 5 Write RAM select okay -> 0 Main
		7,		// 6 Read ROM select 4MB -> 7 Read ROM select 8MB
		8,		// 7 Read ROM select 8MB -> 8 Read ROM select 16MB
		9,		// 8 Read ROM select 16MB -> 9 Read ROM select 32MB
		3,		// 9 Read ROM select 32MB -> 3 Read ROM select 2MB
		0,		// 10 Execute Read ROM 2MB -> 0 Main
		0,		// 11 Execute Read ROM 4MB -> 0 Main
		0,		// 12 Execute Read ROM 8MB -> 0 Main
		0,		// 13 Execute Read ROM 16MB -> 0 Main
		0,		// 14 Execute Read ROM 32MB -> 0 Main
		16,		// 15 Read RAM select Flash -> 16 Read RAM select F/SRAM
		4,		// 16 Read RAM select F/SRAM -> 4 Read RAM select EEPROM 512
		0,		// 17 Execute RAM read EEPROM 8k -> 0 Main
		0,		// 18 Execute RAM read	EEPROM 512 -> 0 Main
		0,		// 19 Execute RAM write EEPROM 512 -> 0 Main
		0,		// 20 Execute RAM read Flash -> 0 Main
		0,		// 21 Execute RAM read F/SRAM -> 0 Main
		0,		// 22 Execute RAM write EEPROM 8k -> 0 Main
		0,		// 23 Execute RAM write Flash -> 0 Main
		0,		// 24 Execute RAM write F/SRAM -> 0 Main
		26,		// 25 Write RAM select EEPROM 512 -> 26 Write RAM select EEPROM 8k
		27,		// 26 Write RAM select EEPROM 8k -> 27 Write RAM select Flash
		28,		// 27 Write RAM select Flash -> 28 Write RAM select F/SRAM
		25};	// 28 Write RAM select F/SRAM -> 25 Write RAM select EEPROM 512
	
	/* IO Initialization */
	GBA_LED_INIT();
	GBA_LED_ON();
	GBA_B1_INIT();
	GBA_B2_INIT();
		
	/* GBA Higher Function Initialization */
	gba_init();
	
	/* Prepare Sleep Function */
	//set_sleep_mode(SLEEP_MODE_IDLE);
	
	/* Continual Loop */
	/* This state machine cycles through options indicated */
	/* by user input about course of computation action.   */
	/* State machine states:			*/
	/* 0: Introductory screen			*/
	/* 1: Read from ROM select			*/
	/* 2: Read RAM select EEPROM 8k		*/
	/* 3: Read ROM select 2MB			*/
	/* 4: Read RAM select EEPROM 512	*/
	/* 5: Write RAM select okay			*/
	/* 6: Read ROM select 4MB			*/
	/* 7: Read ROM select 8MB			*/
	/* 8: Read ROM select 16MB			*/
	/* 9: Read ROM select 32MB			*/
	/* 10: Execute Read ROM 2MB			*/
	/* 11: Execute Read ROM 4MB			*/
	/* 12: Execute Read ROM 8MB			*/
	/* 13: Execute Read ROM 16MB		*/
	/* 14: Execute Read ROM 32MB		*/
	/* 15: Read RAM select Flash		*/
	/* 16: Read RAM select F/SRAM		*/
	/* 17: Execute RAM read EEPROM 8k	*/
	/* 18: Execute RAM read	EEPROM 512	*/
	/* 19: Execute RAM write EEPROM 512	*/
	/* 20: Execute RAM read Flash		*/
	/* 21: Execute RAM read F/SRAM		*/
	/* 22: Execute RAM write EEPROM 8k	*/
	/* 23: Execute RAM write Flash		*/
	/* 24: Execute RAM write F/SRAM		*/
	/* 25: Write RAM select EEPROM 512	*/
	/* 26: Write RAM select EEPROM 8k	*/
	/* 27: Write RAM select Flash		*/
	/* 28: Write RAM select F/SRAM		*/
	
    while(1) {
		
		if (new_op) {
			if (select==0) {
				lcd_clrscr();
				lcd_puts("Welcome! Select:");
				lcd_gotoxy(0,1);
				lcd_puts("< Read   Write >");
				new_op = 0;
			} else if (select==1) {
				lcd_clrscr();
				lcd_puts("Read ROM or RAM?");
				lcd_gotoxy(0,1);
				lcd_puts("< ROM      RAM >");
				new_op = 0;
			} else if (select==2) {
				lcd_gotoxy(7,1);
				lcd_puts("8");
				new_op = 0;
			} else if (select==3) {
				lcd_clrscr();
				lcd_puts("What size ROM?");
				lcd_gotoxy(0,1);
				lcd_puts("< 2MB     Next >");
				new_op = 0;
			} else if (select==4) {
				lcd_clrscr();
				lcd_puts("Read Cart RAM:");
				lcd_gotoxy(0,1);
				lcd_puts("< EPROM1k Next >");
				new_op = 0;
			} else if (select==5) {
				lcd_clrscr();
				lcd_puts("Write 'up.sav'?");
				lcd_gotoxy(0,1);
				lcd_puts("< Yes       No >");
				new_op = 0;
			} else if (select==6) {
				lcd_gotoxy(2,1);
				lcd_puts("4");
				new_op = 0;
			} else if (select==7) {
				lcd_gotoxy(2,1);
				lcd_puts("8");
				new_op = 0;
			} else if (select==8) {
				lcd_gotoxy(1,1);
				lcd_puts("16");
				new_op = 0;
			} else if (select==9) {
				lcd_gotoxy(1,1);
				lcd_puts("32");
				new_op = 0;
			} else if (select==10) {
				// ROM rip 2MB
				gba_read_rom(0x00200000);
				select = 0;
				new_op = 1;
			} else if (select==11) {
				// ROM rip 4MB
				gba_read_rom(0x00400000);
				select = 0;
				new_op = 1;
			} else if (select==12) {
				// ROM rip 8MB
				gba_read_rom(0x00800000);
				select = 0;
				new_op = 1;
			} else if (select==13) {
				// ROM rip 16MB
				gba_read_rom(0x01000000);
				select = 0;
				new_op = 1;
			} else if (select==14) {
				// ROM rip 32MB
				gba_read_rom(0x02000000);
				select = 0;
				new_op = 1;
			} else if (select==15) {
				lcd_gotoxy(2,1);
				lcd_puts("Flash  ");
				new_op = 0;
			} else if (select==16) {
				lcd_gotoxy(2,1);
				lcd_puts("F/SRAM");
				new_op = 0;
			} else if (select==17) {
				// RAM write EEPROM 8k
				gba_read_ram(4);
				select = 0;
				new_op = 1;
			} else if (select==18) {
				// RAM read EEPROM 512
				gba_read_ram(3);
				select = 0;
				new_op = 1;
			} else if (select==19) {
				// RAM write EEPROM 512
				gba_write_ram(3);
				select = 0;
				new_op = 1;
			} else if (select==20) {
				gba_read_ram(0);
				select = 0;
				new_op = 1;
			} else if (select==21) {
				gba_read_ram(2);
				select = 0;
				new_op = 1;
			} else if (select==22) {
				gba_write_ram(4);
				select = 0;
				new_op = 1;
			} else if (select==23) {
				gba_write_ram(0);
				select = 0;
				new_op = 1;
			} else if (select==24) {
				gba_write_ram(2);
				select = 0;
				new_op = 1;
			} else if (select==25) {
				lcd_clrscr();
				lcd_puts("Write Cart RAM:");
				lcd_gotoxy(0,1);
				lcd_puts("< EPROM1k Next >");
				new_op = 0;
			} else if (select==26) {
				lcd_gotoxy(7,1);
				lcd_puts("8");
				new_op = 0;
			} else if (select==27) {
				lcd_gotoxy(2,1);
				lcd_puts("Flash  ");
				new_op = 0;
			} else if (select==28) {
				lcd_gotoxy(2,1);
				lcd_puts("F/SRAM");
				new_op = 0;
			}
			_delay_ms(200);		
		}		
		
		if (GBA_B1_CHECK() || GBA_B2_CHECK()) {
			_delay_ms(20);
			if (GBA_B1_CHECK()) {
				new_op = 1;
				select = next_sel_b1[select];
			}
			else if (GBA_B2_CHECK()) {
				new_op = 1;
				select = next_sel_b2[select];
			}
		}
		_delay_ms(80);	
    }
}