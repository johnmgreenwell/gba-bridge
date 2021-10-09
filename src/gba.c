/*-------------------------------------------------------------/
/
/	FILE: gba.c
/
/	VERSION: 1.0 -- 31 March 2014
/
/	AUTHOR: John Greenwell
/
/	DESCRIPTION:
/	This c file contains functions referenced in gba.h for
/	Atmel AVR GBA communication.
/
/	NOTES:
/	See gba.h for more information.
/
/-------------------------------------------------------------*/
#include "gba.h"


/* Global Variables */
FILE uart_str = FDEV_SETUP_STREAM(uart_putch, uart_getch, _FDEV_SETUP_RW);
volatile uint8_t byte = 0;
volatile uint8_t b1_flg = 0;
volatile uint8_t b2_flg = 0;
FATFS Fatfs;		/* File system object */
FIL Fil;			/* File object */
BYTE Buff[128];		/* File read buffer */


/* UART RX ISR */
ISR(USART0_RX_vect) {
	cli();
	byte = uart_getch(&uart_str);
	sei();
}


/* Button Pin Change Interrupt */
ISR(PCINT0_vect) {
	cli();
	_delay_ms(20);	// Debounce time
	if (GBA_B1_CHECK()) { b1_flg = 1; }
	else if (GBA_B2_CHECK()) { b2_flg = 1; }
	sei();
}


/* Return State of b1_flg */
uint8_t get_b1_flg() {
	return b1_flg;
}

/* Return State of b2_flg */
uint8_t get_b2_flg() {
	return b2_flg;
}

/* Reset b1_flg */
void reset_b1_flg() {
	b1_flg = 0;
}

/* Reset b2_flg */
void reset_b2_flg() {
	b2_flg = 0;
}


/* Reverse the Bits in a Byte */
uint8_t flipByte(uint8_t num) {
	num = (num & 0xF0) >> 4 | (num & 0x0F) << 4;
	num = (num & 0xCC) >> 2 | (num & 0x33) << 2;
	num = (num & 0xAA) >> 1 | (num & 0x55) << 1;
return num;
}


/* SD Error All-Stop */
void gba_error(FRESULT rc) {
	char buf[3] = {0,0,0};
	lcd_clrscr();
	lcd_puts("ERROR: ");
	itoa(rc,buf,10);
	for (uint8_t i=0;i<3;i++) { lcd_putc(buf[i]); }
	for(;;); // End of the road
}


/* All GBA Data Ports as Inputs */
void gba_ports_in() {
	HIGH_DDR = 0;
	HIGH_PORT = 0;
	MID_DDR = 0;
	MID_PORT = 0;
	LOW_DDR = 0;
	LOW_PORT = 0;
}


/* All GBA Data Ports as Outputs */
void gba_ports_out() {
	HIGH_DDR = 0xFF;
	MID_DDR = 0xFF;
	LOW_DDR = 0xFF;
}


/* Initialize GBA IO */
void gba_init() {
	DDRD |= (1<<3);				// NOTE: Ensure SD CS pin initialized
	PORTD |= (1<<3);			// NOTE: Ensure SD CS pin high
	WR_DDR |= (1<<WR_PIN);		// Write pin set to output
	WR_PORT |= (1<<WR_PIN);		// Ensure write pin high initially
	RD_DDR |= (1<<RD_PIN);		// Read pin set to output
	RD_PORT |= (1<<RD_PIN);		// Ensure read pin high initially
	CS_DDR |= (1<<CS_PIN);		// CS pin set to output
	CS_PORT |= (1<<CS_PIN);		// Ensure CS high initially
	CS2_DDR |= (1<<CS2_PIN);	// CS2 pin set to output
	CS2_PORT |= (1<<CS2_PIN);	// Ensure CS2 high initially
	gba_ports_in();				// All GBA data pins input initially
	uart_init();				// Initialise UART
	lcd_init(LCD_DISP_ON);		// Initialise LCD
	f_mount(0, &Fatfs);			// Register SD volume work area (never fails)
	stdin = stdout = &uart_str;
}


/* Latch ADDR to 24-bit Line */
void gba_latch_rom_addr(uint32_t addr) {
	uint8_t byte_h, byte_m, byte_l;
	gba_ports_out();
	byte_l = (uint8_t)(addr & 0x000000FF);
	byte_m = flipByte((uint8_t)((addr & 0x0000FF00)>>8));
	byte_h = flipByte((uint8_t)((addr & 0x00FF0000)>>16));
	LOW_PORT = byte_l;
	MID_PORT = byte_m;
	HIGH_PORT = byte_h;
	CS_HIGH(); _delay_us(2); CS_LOW();
	gba_ports_in();
}


/* Prepare ports to RAM latch */
void gba_latch_ram_init() {
	LOW_DDR = 0xFF;
	MID_DDR = 0xFF;
	HIGH_DDR = 0;
	HIGH_PORT = 0;
}


/* End port preparation to RAM latch */
void gba_latch_ram_end() {
	MID_DDR = 0;
	MID_PORT = 0;
	LOW_DDR = 0;
	LOW_PORT = 0;
	CS2_HIGH();
}


/* Latch ADDR to 16-bit line */
void gba_latch_ram_addr(uint16_t addr) {
	uint8_t byte_m, byte_l;
	byte_l = (uint8_t)(addr & 0x00FF);
	byte_m = flipByte((uint8_t)((addr & 0xFF00)>>8));
	LOW_PORT = byte_l;
	MID_PORT = byte_m;
}


/* Read 16-bit Data from GBA Cartridge */
void gba_read_rom_data(uint16_t * data_ptr) {
	// Note at the end that 'h' and 'l' are actually switched
	uint8_t data_l; uint8_t data_h;
	data_h = MID_PIN;
	data_l = LOW_PIN;
	data_h = flipByte(data_h);
	*data_ptr = ((uint16_t)(data_h)) & 0x00FF;
	*data_ptr |= ((uint16_t)(data_l)<<8);
}


/* Read 8-bit Data from GBA RAM */
void gba_read_ram_data(uint8_t * data_ptr) {
	uint8_t data_h;
	data_h = HIGH_PIN;
	*data_ptr = flipByte(data_h);
}


/* Prepare 8-bit Data to the GBA RAM */
void gba_write_ram_data(uint8_t data) {
	uint8_t data_h;
	data_h = flipByte(data);
	HIGH_PORT = data_h;
}


/* Obtain Chip ID from Flash RAM to Check Type*/
uint8_t gba_check_ram() {
	uint8_t ram_type = 0;
	uint8_t code = 0;
	// Look for manufacturer data 
	RD_HIGH(); HIGH_DDR = 0xFF;
	gba_latch_ram_addr(0x5555);
	gba_write_ram_data(0xAA); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	gba_latch_ram_addr(0x2AAA);
	gba_write_ram_data(0x55); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	gba_latch_ram_addr(0x5555);
	gba_write_ram_data(0x90); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	HIGH_DDR = 0x00; RD_LOW();
	gba_latch_ram_addr(0x0000);
	_delay_us(2);
	gba_read_ram_data(&code);
	RD_HIGH(); HIGH_DDR = 0xFF;
	gba_latch_ram_addr(0x5555);
	gba_write_ram_data(0xAA); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	gba_latch_ram_addr(0x2AAA);
	gba_write_ram_data(0x55); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	gba_latch_ram_addr(0x5555);
	gba_write_ram_data(0xF0); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	if (code==0xBF) { ram_type = 0; /* SST */ }
	else if (code==0x1F) { ram_type = 1; /* Atmel */ }
	_delay_ms(1);
	return ram_type;
} 


/* Erase Chip SST FLash RAM (requires all IO setup outside function) */
void gba_sst_flash_ram_erase_chip() {
	gba_latch_ram_addr(0x5555);
	gba_write_ram_data(0xAA); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	gba_latch_ram_addr(0x2AAA);
	gba_write_ram_data(0x55); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	gba_latch_ram_addr(0x5555);
	gba_write_ram_data(0x80); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	gba_latch_ram_addr(0x5555);
	gba_write_ram_data(0xAA); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	gba_latch_ram_addr(0x2AAA);
	gba_write_ram_data(0x55); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
	gba_latch_ram_addr(0x5555);
	gba_write_ram_data(0x10); _delay_us(1);
	WR_LOW(); _delay_us(2); WR_HIGH();
}


/* Write Byte SST FLash RAM (requires all IO setup outside function) */
void gba_sst_flash_ram_write_byte(uint8_t addr, uint8_t data) {
	 gba_latch_ram_addr(0x5555);
	 gba_write_ram_data(0xAA); _delay_us(1);
	 WR_LOW(); _delay_us(2); WR_HIGH();
	 gba_latch_ram_addr(0x2AAA);
	 gba_write_ram_data(0x55); _delay_us(1);
	 WR_LOW(); _delay_us(2); WR_HIGH();
	 gba_latch_ram_addr(0x5555);
	 gba_write_ram_data(0xA0); _delay_us(1);
	 WR_LOW(); _delay_us(2); WR_HIGH();
	 gba_latch_ram_addr(addr);
	 gba_write_ram_data(data); _delay_us(1);
	 WR_LOW(); _delay_us(2); WR_HIGH();
	 _delay_us(60);
 }
 
 
 /* Prepare pins for EEPROM communication */
 void gba_latch_eeprom_start() {
	 A23_OUTPUT(); A0_OUTPUT(); CS_LOW(); CS2_LOW(); RD_HIGH(); WR_LOW();
	 A23_HIGH();
 }
 
 
 /* Prepare pins for EEPROM serial read */
 void gba_latch_eeprom_read() {
	 A23_OUTPUT(); A0_INPUT(); CS_LOW(); CS2_LOW(); WR_HIGH(); 
	 _delay_us(1);
	 RD_LOW();
	 A23_HIGH();
 }
 
 
 /* End preparation of pins for EEPROM communication */
 void gba_latch_eeprom_end() {
	 A23_LOW();
	 A23_INPUT(); A0_INPUT(); CS_HIGH(); CS2_HIGH(); RD_HIGH(); WR_HIGH();
 }
 
 
  /* EEPROM Write Byte Function */
void gba_eeprom_write_byte(uint8_t byte) {
	for (uint8_t i=0;i<8;i++) {
		if (byte & (1<<(7-i))) { A0_HIGH(); }
		else { A0_LOW(); }
		_delay_us(1); WR_HIGH(); _delay_us(1); WR_LOW();
	}
  }
  
 
 /* EEPROM Read Byte Function */
 uint8_t gba_eeprom_read_byte() {
	 uint8_t byte = 0;
	 for (uint8_t i=0;i<8;i++) {
		RD_HIGH(); _delay_us(2);
		if (A0_CHECK()) { byte |= (1<<(7-i)); }
		RD_LOW(); _delay_us(4);
	}
	return byte;
 }
 
 void gba_eeprom_write_addr(uint16_t addr, uint8_t eeprom_type) {
	 uint8_t high_byte, low_byte = 0;
	 if (eeprom_type) {
		high_byte = ((uint8_t)((addr & 0xFF00)>>8)) | (1<<7);
		low_byte = (uint8_t)(addr & 0x00FF);
		gba_eeprom_write_byte(high_byte);
		gba_eeprom_write_byte(low_byte);
	 } else {
		low_byte = ((uint8_t)(addr & 0x00FF)) | (1<<7);
		gba_eeprom_write_byte(low_byte);
	 }		 
 }


/* Check that the ROM is OK after it is read */
void gba_checksum_rom() {
	//uint8_t chk[15] = ".\0\0ê$ÿ®Qiš¢!=„‚";
	FRESULT rc;
	rc = f_open(&Fil, "rom_rip.gba", FA_READ | FA_OPEN_EXISTING);
	if (rc) { gba_error(rc); }
}


/* */
void gba_print_return_menu() {
	lcd_gotoxy(0,1);
	lcd_puts("Back to menu ");
	for (uint8_t i=0;i<3;i++) {
		_delay_ms(550);
		lcd_putc('.');
	}
}


/* Read ROM Bank Function */
void gba_read_rom(uint32_t size) {
	uint32_t addr = 0;		// Start address at 0
	uint16_t data_sector[32] = {};
	uint16_t notch = size/4096;
	uint8_t cnt = 0;
	uint8_t data_cnt = 0;
	uint16_t cycle = 0;
	uint8_t lcd_pos = 0;
	FRESULT rc;				// Result code
	lcd_clrscr();
	lcd_puts("ROM Dumping...");
	lcd_gotoxy(0,1);
	lcd_puts(" --|        |-- ");
	rc = f_open(&Fil, "rom_rip.gba", FA_WRITE | FA_CREATE_ALWAYS);
	if (rc) { gba_error(rc); }
	CS_LOW(); _delay_us(1);
	gba_latch_rom_addr(addr); _delay_us(1);
	while (addr<size) {
		if ((GBA_B2_CHECK()) && (addr > 100)) { _delay_ms(20);
			if (GBA_B2_CHECK()) { break; }
		}
		if (data_cnt == 31) {
			data_cnt = 0;
			for (uint8_t i=0;i<32;i++) {
				f_putc((uint8_t)(data_sector[i]>>8),&Fil);
				f_putc((uint8_t)data_sector[i],&Fil);
			}			
		}
		//gba_latch_rom_addr(addr/2);
		RD_LOW(); _delay_us(10);
		gba_read_rom_data(data_sector+data_cnt);
		RD_HIGH();
		addr+=2;
		if (cnt==255) {
			if (cycle == notch) {
				cycle = 0;
				lcd_gotoxy(lcd_pos+4,1);
				lcd_putc('#');
				lcd_pos++;
			}
			if (GBA_CHECK_LED()) { GBA_LED_OFF(); } 
			else { GBA_LED_ON(); }
			cycle++;
		}		
		cnt++; data_cnt++;
	}
	CS_HIGH();
	rc = f_close(&Fil);
	if (rc) { gba_error(rc); }
	lcd_clrscr();
	lcd_puts("ROM Dump Ended.");
	gba_print_return_menu();
}


/* Read Flash RAM or SRAM Function */
void gba_read_ram(uint8_t ram_type) {
	uint16_t addr = 0;
	uint8_t data = 0;
	uint8_t cnt = 0;
	uint8_t cycle = 0;
	uint8_t lcd_pos = 0;
	uint16_t size = 0xFFFF;
	uint16_t notch = size/4096;
	FRESULT rc;
	lcd_clrscr();
	lcd_puts("RAM Dumping...");
	lcd_gotoxy(0,1);
	lcd_puts(" --|        |-- ");
	rc = f_open(&Fil, "ram_rip.sav", FA_WRITE | FA_CREATE_ALWAYS);
	if (rc) { gba_error(rc); }
	GBA_LED_ON();
	if (ram_type > 2) {
		size = 512;
		if (ram_type == 4) { size = 8192; }
		notch = size/2048;
		gba_latch_eeprom_start(); _delay_us(5);
		gba_eeprom_write_byte(0xC0);
		gba_eeprom_write_byte(0x00);
		gba_latch_eeprom_end();
		_delay_us(20);
		gba_latch_eeprom_read();
		_delay_us(3);
		for (uint8_t i=0;i<4;i++) {
			_delay_us(1); RD_HIGH(); _delay_us(1); RD_LOW();
		}
	}
	else {
		gba_latch_ram_init();
		HIGH_DDR = 0x00; CS2_LOW(); WR_HIGH(); RD_LOW();
	}	
	// Read Loop
	while (addr < size) {
		if (ram_type > 2) {
			data = gba_eeprom_read_byte();
		} else {
			gba_latch_ram_addr(addr); _delay_us(1);
			if (ram_type == 2) { RD_HIGH(); _delay_us(2); RD_LOW(); }
			gba_read_ram_data(&data);
		}		
		f_putc((uint8_t)data,&Fil);
		addr++;
		if (cnt == 255) {
			if (cycle == notch) {
				cycle = 0;
				lcd_gotoxy(lcd_pos+4,1);
				lcd_putc('#');
				lcd_pos++;
			}
			if (GBA_CHECK_LED()) { GBA_LED_OFF(); }
			else { GBA_LED_ON(); }
			cycle++;
		}
		cnt++;
	}
	if (ram_type < 3) {
		gba_latch_ram_addr(addr); _delay_us(1);
		if (ram_type == 2) { RD_HIGH(); _delay_us(2); RD_LOW(); }
		gba_read_ram_data(&data);
		RD_HIGH();
		f_putc((uint8_t)data,&Fil);	// Write final byte to SD card
	}	
	GBA_LED_OFF();
	CS2_HIGH();
	gba_latch_eeprom_end();
	gba_latch_ram_end();
	rc = f_close(&Fil);
	if (rc) { gba_error(rc); }
	lcd_clrscr();
	lcd_puts("RAM Dump Ended.");
	gba_print_return_menu();
}


/* Write RAM Function */
void gba_write_ram(uint8_t ram_type) {
	// Local variables
	uint16_t addr = 0;
	uint16_t size = 0xFFFF;
	uint16_t notch = size/4096;
	uint8_t data = 0;
	uint8_t eeprom_type = 0;
	uint8_t data_sector[128] = {};
	uint8_t data_array[8] = {};
	uint8_t add_byte = 0;
	uint8_t buf = 0;
	uint8_t cnt = 0;
	uint8_t cycle = 0;
	uint8_t lcd_pos = 0;
	FRESULT rc;
	// Display write begin
	lcd_clrscr();
	lcd_puts("RAM Writing...");
	lcd_gotoxy(0,1);
	lcd_puts(" --|        |-- ");
	gba_latch_ram_init();
	if (ram_type == 0) {
		ram_type = gba_check_ram();
	}	
	if (ram_type == 0) {
		CS2_LOW(); RD_HIGH(); WR_HIGH(); HIGH_DDR = 0xFF;
		gba_sst_flash_ram_erase_chip();
		for (uint8_t i=0;i<2;i++) { _delay_ms(1000); }	// Delay
	} else if (ram_type == 2) {
		size = 0x7FFF;
		notch = size/4096;
	}
	CS2_HIGH(); HIGH_DDR = 0x00;
	gba_latch_ram_end();
	// Access the file to be written
	rc = f_open(&Fil, "up.sav", FA_READ);
	if (rc) { gba_error(rc); }
	// Initiate IO to RAM access
	gba_latch_ram_init();
	CS2_LOW(); RD_HIGH(); WR_HIGH(); HIGH_DDR = 0xFF;
	_delay_us(2);
	if (ram_type == 2) { 
		WR_LOW(); 
		// Test accessibility of RAM first byte
		for (uint8_t i=0;i<8;i++) {
			gba_write_ram_data(data_array[i]); _delay_us(1);
			WR_HIGH(); _delay_us(2);
			gba_latch_ram_addr((uint16_t)i); _delay_us(2);
			WR_LOW(); _delay_us(1);
		}
		WR_HIGH(); HIGH_DDR = 0x00;
		_delay_us(25); RD_LOW(); _delay_us(25);
		gba_latch_ram_addr(0x00); _delay_us(1);
		RD_HIGH(); _delay_us(2); RD_LOW();
		gba_read_ram_data(&data); _delay_us(2);
		RD_HIGH(); HIGH_DDR = 0xFF;
		if (data != data_array[0]) { add_byte = 1; }
		WR_LOW(); _delay_us(5);
	} else if (ram_type > 2) {
		if (ram_type == 4) { size = 8192; notch = 4; eeprom_type = 1; }
		else { size = 512; notch = 1; }	
	}
	// Write Loop
	while (addr < size) {
		if ((ram_type != 1) && !(ram_type > 2)) { f_read(&Fil,&data,1,&buf); }
		if (ram_type == 0) {
			gba_sst_flash_ram_write_byte(addr,data);
		} else if (ram_type == 1) {
			if (addr % 128 == 0) {
				f_read(&Fil,&data_sector,128,&buf);
				gba_latch_ram_addr(0x5555);
				gba_write_ram_data(0xAA); _delay_us(1);
				WR_LOW(); _delay_us(2); WR_HIGH();
				gba_latch_ram_addr(0x2AAA);
				gba_write_ram_data(0x55); _delay_us(1);
				WR_LOW(); _delay_us(2); WR_HIGH();
				gba_latch_ram_addr(0x5555);
				gba_write_ram_data(0xA0); _delay_us(1);
				WR_LOW(); _delay_us(2); WR_HIGH();
				_delay_us(60);
				for (uint8_t i=0;i<128;i++) {
					gba_latch_ram_addr(addr+i);
					gba_write_ram_data(data_sector[i]); _delay_us(1);
					WR_LOW(); _delay_us(2); WR_HIGH();
				}
				_delay_ms(40);
			}
		} else if (ram_type == 2) {
			gba_write_ram_data(data); _delay_us(1);
			WR_HIGH(); _delay_us(2);
			gba_latch_ram_addr(addr + add_byte); _delay_us(2); 
			WR_LOW(); _delay_us(1);
		} else {
			if (addr % 8 == 0) {
				f_read(&Fil,&data_array,8,&buf);
				gba_latch_eeprom_start(); _delay_us(5);
				gba_eeprom_write_addr(addr/8,eeprom_type);
				_delay_us(5);
				for (uint8_t i=0;i<8;i++) {
					gba_eeprom_write_byte(data_array[i]);
				}
				A0_LOW(); _delay_us(1); WR_HIGH(); _delay_us(1); WR_LOW();
				gba_latch_eeprom_end(); _delay_ms(8);
			}				
		}
		addr++;
		if (cnt==255) {
			if (cycle == notch) {
				cycle = 0;
				lcd_gotoxy(lcd_pos+4,1);
				lcd_putc('#');
				lcd_pos++;
			}
			if (GBA_CHECK_LED()) { GBA_LED_OFF(); }
			else { GBA_LED_ON(); }
			cycle++;
		}
		cnt++;
	}
	// Write final byte if necessary
	if (ram_type != 1 && !(ram_type > 2)) { f_read(&Fil,&data,1,&buf); }
	if (ram_type == 0) {
		gba_sst_flash_ram_write_byte(addr,data);
	}
	HIGH_DDR = 0x00; CS2_HIGH(); WR_HIGH(); RD_HIGH();
	GBA_LED_OFF();
	gba_latch_eeprom_end();
	gba_latch_ram_end();
	rc = f_close(&Fil);
	if (rc) { gba_error(rc); }
	lcd_clrscr();
	lcd_puts("RAM Write Ended.");
	gba_print_return_menu();
}