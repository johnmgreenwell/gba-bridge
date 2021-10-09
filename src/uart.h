/*------------------------------------------------------------------------/
/-------------------------- AVR UART Functions ---------------------------/
/-------------------------------------------------------------------------/
/	AUTHOR
/		John Greenwell
/
/	VERSIONS
/		1.2 -- 06 March 2013
/		1.1 -- 30 January 2013 
/		1.0 -- 17 January 2013
/-------------------------------------------------------------------------/
/	DESCRIPTION
/		This include file provides access to simple Atmel AVR UART 
/		(universal asynchronous receive transmit) initializations and 
/		functions. If UART_TERM is value 1 below, simple terminal 
/		operability is also compiled.
/
/		Ensure that the desired baud rate and MCU clock speed are correctly
/		defined below.
/
/		The including main .c file should include this line:
/
/	// Assign I/O stream to UART
/	FILE uart_str = FDEV_SETUP_STREAM(uart_putch, uart_getch, _FDEV_SETUP_RW);
/
/		If terminal functions are used, the including main .c file should have 
/		these volatile global variables and UART receive-ready ISR:
/
/			// Global variables for UART terminal ISR
/			volatile unsigned char r_char = 0;
/			volatile unsigned char r_flag = 0;
/
/			// UART character-ready ISR
/			ISR(USART_RX_vect) {
/				cli();
/				r_char = UDR0;	// Get character
/
/				// Set appropriate flags
/				if (r_char == '\b') {
/					r_flag |= (1<<BSMB);
/				} else if (r_char==ESC) {
/					r_flag |= (1<<ESMB);
/				} else if (r_char == '\r') {
/					r_flag |= (1<<ERMB);
/				}
/				r_flag |= (1<<DRMB);
/				sei();
/			}
/
/		If terminal functions are used, the TERMINAL structure:
/
/			// Variable declaration for UART terminal
/			TERMINAL r_term;
/
/		And initialization (using &r_term parameter only when terminal used):
/
/			uart_init(&r_term);
/
/		Obtaining user input (global interrupts must already be enabled):
/
/			uart_term_get(&r_term,&r_char,&r_flag);
/
/		Once this has been done, the TERMINAL structure r_term will now
/		contain the user input as follows:
/
/			r_term.cmd --> character array containing main command
/			r_term.flgs --> character array containing all flags called
/			r_terms.prms --> character array containing single byte parameters
/-------------------------------------------------------------------------/
/	NOTES
/		All of the interpretation of the values left after uart_term_get() 
/		has been called must be handled in the main .c file. One way to 
/		accomplish this more cleanly is to call a function which sets flags 
/		according to input and then a simpler conditional scheme may be 
/		achieved in main().
/
/		While awaiting user terminal input, idle sleep mode is used.
/-------------------------------------------------------------------------/
/------------------------------------------------------------------------*/

#ifndef UART_H_
#define UART_H_

/*--------------------------------------------------------------------------*/
/* AVR_UART Configuration                                                   */
/*--------------------------------------------------------------------------*/

/* F_CPU Definition */
#ifndef F_CPU
#define F_CPU		16000000UL
#endif
/* ASCII ESC Definition */
#define ESC			27

/* UART Baud Rate */
#define BAUD_RATE	19200UL

/* UART Receive Interrupt Enable */
#define UART_RX_INT	1	/* 0:disabled or 1:enabled */

/* UART Echo on Receive */
#define RX_ECHO		1	/* 0:disabled or 1:enabled */

/* UART Terminal Functionality */
#define UART_TERM	0	/* 0:disabled or 1:enabled */

/*--------------------------------------------------------------------------*/
/* Available Functions                                                      */
/*--------------------------------------------------------------------------*/
#if UART_TERM==0
void uart_init();
#endif
int uart_putch(char data, FILE *stream);
int uart_getch(FILE *stream);
void uart_ansi_cls();
void uart_ansi_disp(unsigned char attr);
void uart_ansi_cursor(unsigned char row,unsigned char col);
void toUpperStr(unsigned char * sPtr);
#if UART_TERM  /* UART Terminal Functions */
void uart_init(TERMINAL * rt);
void uart_term_get(TERMINAL * rt,volatile unsigned char * rc,volatile unsigned char * rf);
void uart_term_act(TERMINAL * rt,volatile unsigned char * rc,volatile unsigned char * rf);
void uart_term_eval(TERMINAL * rt);
#endif


#if UART_TERM==1
/*--------------------------------------------------------------------------*/
/* INTERNAL Terminal Operation Options -- Change Only with Care             */
/*--------------------------------------------------------------------------*/

/* UART Terminal defines */
#define BUF_SIZE	40
#define CMD_SIZE	10
#define NFLG_SIZE	6
#define NPRM_SIZE	6	// NOTE: to change, variables in uart.c must be added/removed
#define PRM_SIZE	10
#define RBH_SIZE	5	// Number of accessible past commands = RBH_SIZE-1
						// NOTE: to change, variables below and uart.c must be added/removed

/* ASCII defines */
#define SPACE		32
#define O_BRACK		91
#define L_MARK		68	// LEFT:	1B5B44 = <ESC>[D
#define UP_MARK		65	// UP:		1B5B41 = <ESC>[A
#define R_MARK		67	// RIGHT:	1B5B43 = <ESC>[C
#define D_MARK		66	// DOWN:	1B5B42 = <ESC>[B

/* UART interrupt flag bit defines */
#define	ESMB	7	// Escape-mark-bit
#define BSMB	6	// Backspace-mark-bit
#define ERMB	5	// Excess-return-mark-bit
#define DRMB	4	// Data-ready-mark-bit


/*--------------------------------------------------------------------------*/
/* TERMINAL Typedef, Contains rbh char ** of Past Cmd Buffer Pointers       */
/*--------------------------------------------------------------------------*/
typedef struct {
	unsigned char rb[BUF_SIZE+1];
	unsigned char rb1[BUF_SIZE+1];
	unsigned char rb2[BUF_SIZE+1];
	unsigned char rb3[BUF_SIZE+1];
	unsigned char rb4[BUF_SIZE+1];
	unsigned char * rbh[RBH_SIZE];
	unsigned char rhn;

	unsigned char cmd[CMD_SIZE+1];
	unsigned char flgs[NFLG_SIZE+1];
	unsigned char prms[NPRM_SIZE+1];
} TERMINAL;
#endif

#endif /* UART_H_ */