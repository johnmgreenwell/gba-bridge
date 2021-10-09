/*-------------------------------------------------------------/
/
/	FILE: uart.c
/
/	VERSION: 1.2 -- 06 March 2013
/
/	AUTHOR: John Greenwell
/
/	DESCRIPTION:
/	This c file contains functions referenced in uart.h for
/	Atmel AVR UART communication. If UART_TERM is defined in
/	uart.h, simple terminal functionality is also enabled.
/
/	NOTES:
/	See uart.h for more information.
/
/-------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "uart.h"

#if UART_TERM==0
/*---------------------------------------------------------*/
/* UART NON-TERMINAL Initialization                        */
/*---------------------------------------------------------*/
void uart_init(void)
{
	/* Set baud rate */
	UBRR0H = (((F_CPU/BAUD_RATE)/16)-1)>>8;
	UBRR0L = (((F_CPU/BAUD_RATE)/16)-1);

	/* Enable Tx and Rx, optionally RX Interrupt Enable */
#if UART_RX_INT
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
#else
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
#endif

	/* Set Frame: Data 8 Bit, No Parity and 1 Stop Bit */
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

}
#endif

/*---------------------------------------------------------*/
/* UART_PUTCH Function                                     */
/*---------------------------------------------------------*/
int uart_putch(char data, FILE *stream)
{
	if (data == '\n')
	uart_putch('\r', stream);

	while (!(UCSR0A & (1<<UDRE0)));
	UDR0=data;

	return 0;
}

/*---------------------------------------------------------*/
/* UART_GETCH Function                                     */
/*---------------------------------------------------------*/
int uart_getch(FILE *stream)
{
	unsigned char data;

	while (!(UCSR0A & (1<<RXC0)));
	data=UDR0;
#if RX_ECHO == 1
	/* Echo the output back to the terminal */
	uart_putch(data,stream);
#endif

	return data;
}

/*---------------------------------------------------------*/
/* ANSI Escape Sequences                                   */
/*---------------------------------------------------------*/

/* ANSI Clear Screen */
void uart_ansi_cls(void)
{
	// ANSI clear screen: \E[H\E[J
	fputc(ESC,stdout);
	fputc('[',stdout);
	fputc('H',stdout);
	fputc(ESC,stdout);
	fputc('[',stdout);
	fputc('J',stdout);
}

/* ANSI Attribute */
void uart_ansi_disp(unsigned char attr)
{
	// attr = 0 for ANSI turn off all attribute: \E[0m
	fputc(ESC,stdout);
	fputc('[',stdout);
	fputc(attr,stdout);
	fputc('m',stdout);
}

/* ANSI Cursor Move */
void uart_ansi_curs(unsigned char row,unsigned char col)
{
	// ANSI cursor movement: \E%row;%colH
	fputc(ESC,stdout);
	fputc('[',stdout);
	printf("%d",row);
	fputc(';',stdout);
	printf("%d",col);
	fputc('H',stdout);
}

/* ANSI Move Cursor One Space Right */
void uart_ansi_r() {
	// Move cursor one space right: \E[C
	fputc(ESC,stdout);
	fputc('[',stdout);
	fputc('C',stdout);
}

/* ANSI Move Cursor One Space Left */
void uart_ansi_l() {
	// Move cursor one space left: \E[D
	fputc(ESC,stdout);
	fputc('[',stdout);
	fputc('D',stdout);
}

/*---------------------------------------------------------*/
/* Convert String to Uppercase                             */
/*---------------------------------------------------------*/
void toUpperStr(unsigned char * sPtr) {
	do { (*sPtr) = toupper((unsigned char)*sPtr);
	} while(*sPtr++);
}

#if UART_TERM
/*---------------------------------------------------------*/
/* UART TERMINAL Initialization                            */
/*---------------------------------------------------------*/
void uart_init(TERMINAL * rt) {

	/* Prepare contents of TERMINAL */
	rt->rhn = 0;
	rt->rbh[0] = rt->rb;
	rt->rbh[1] = rt->rb1;
	rt->rbh[2] = rt->rb2;
	rt->rbh[3] = rt->rb3;
	rt->rbh[4] = rt->rb4;
	for (unsigned char i = 0; i < RBH_SIZE; i++) {
		*(rt->rbh[i]) = '\0';
	}
	rt->cmd[0] = '\0';
	rt->flgs[0] = '\0';
	rt->prms[0] = '\0';

	/* Set baud rate */
	UBRR0H = (((F_CPU/BAUD_RATE)/16)-1)>>8;
	UBRR0L = (((F_CPU/BAUD_RATE)/16)-1);

	/* Enable Tx and Rx, optionally RX Interrupt Enable */
#if UART_RX_INT
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
#else
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
#endif

	/* Set Frame: Data 8 Bit, No Parity and 1 Stop Bit */
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

}

/*---------------------------------------------------------*/
/* UART TERMINAL Get and Evaluate Input                    */
/*---------------------------------------------------------*/
void uart_term_get(TERMINAL * rt,volatile unsigned char * rc,volatile unsigned char * rf) {

	UCSR0B |= (1<<RXCIE0);	// Enable Rx interrupt
	uart_term_act(rt,rc,rf);
	uart_term_eval(rt);
}

/*---------------------------------------------------------*/
/* UART TERMINAL Emulation                                 */
/*---------------------------------------------------------*/
void uart_term_act(TERMINAL * rt,volatile unsigned char * rc,volatile unsigned char * rf) {

	unsigned char r_ready = 0;	// Used to indicate buffer completion
	unsigned char r_tem = '\0'; // Used assessing arrow keys
	unsigned char rhi = 0;	// Previous input history index
	unsigned char ri = 0;	// Current buffer index

	while(!(r_ready)) {
		if (*rf & (1<<DRMB)) {		// If flag data ready
			UCSR0B &= ~(1<<RXCIE0);	// Stop rec ISR at outset
			if (*rf & (1<<BSMB)) {	// If backspace, emulate it
				if (ri != 0) {
					ri--;
					printf("\b%c\b",SPACE);
				}
			} else if ((*rf & (1<<ERMB)) || (ri >= 40)) {
				printf("\n");		// Send newline
				rt->rb[ri]='\0';    // Zero terminate
				r_ready = 1;		// Signal str ready flag
			} else if (*rf & (1<<ESMB)) {	// If ESC detect
				r_tem = 3;
			} else if (r_tem > 0) {	// Decrement r_tem to remove ESC trailing chars
				r_tem--;
				if (r_tem==1) {
					r_tem=0;	// Now trailing chars eliminated, check arrows
					if (*rc==UP_MARK) {		  // If up arrow pressed
						if (rhi < (RBH_SIZE-1) && *(rt->rbh[rhi+1])) {
							// First remove anything already typed
							rt->rb[ri] = '\0'; // Null cap current contents
							while (ri != 0) {
								printf("\b%c\b",SPACE);
								ri--;
							}
							rhi++;	// Increment and then print previous input
							while (*(rt->rbh[rhi]+ri)) {
								rt->rb[ri] = *(rt->rbh[rhi]+ri);
								fputc(rt->rb[ri],stdout);
								ri++;
							}
						}
					} else if (*rc==D_MARK) { // If down arrow pressed
						if (rhi > 0) {
							// First remove anything already typed
							rt->rb[ri] = '\0'; // Null cap current contents
							while (ri != 0) {
								printf("\b%c\b",SPACE);
								ri--;
							}
							rhi--;	// Decrement and then print previous input
							while (*(rt->rbh[rhi]+ri)) {
								rt->rb[ri] = *(rt->rbh[rhi]+ri);
								fputc(rt->rb[ri],stdout);
								ri++;
							}
						}
					} else if (*rc==L_MARK) { // If left arrow pressed
					} else if (*rc==R_MARK) { // If right arrow pressed
					} else {	// If no arrow key pressed, just print char
						rt->rb[ri]=*rc;	// Store the char
						fputc(*rc,stdout);	// Print the char
						ri++;
					}
				}
			} else {	// Else if normal char
				rt->rb[ri]=*rc;	// Store the char
				fputc(*rc,stdout);	// Print the char
				ri++;
			}
			// Reset flags, ISR, sleep between each character retrieval from user
			*rf = 0x00;				// Reset ISR-handled flags
			if (!r_ready) {
				UCSR0B |= (1<<RXCIE0);  // Restart Rx ISR
				sleep_enable();			// Enable sleep
				sleep_cpu();			// Sleep
				sleep_disable();		// Disable sleep
			}
		}
	}
	// Cycle stored past inputs; each individual char moved
	if (rt->rb[0] != '\0') {
		for (unsigned char i = RBH_SIZE-1; i > 0; i--) {
			unsigned char j = 0;
			while(*(rt->rbh[i-1]+j)) {
				*(rt->rbh[i]+j) = *(rt->rbh[i-1]+j);
				j++;
			}
			*(rt->rbh[i]+j) = '\0';
		}
	}
}

/*---------------------------------------------------------*/
/* UART TERMINAL Evaluation; Stores Cmds, Flags, Params    */
/*---------------------------------------------------------*/
void uart_term_eval(TERMINAL * rt) {
	/* NOTE: rpx must exist such that x = NPRM_SIZE-1 */
	unsigned char rp0[PRM_SIZE+1] = {};
	unsigned char rp1[PRM_SIZE+1] = {};
	unsigned char rp2[PRM_SIZE+1] = {};
	unsigned char rp3[PRM_SIZE+1] = {};
	unsigned char rp4[PRM_SIZE+1] = {};
	unsigned char rp5[PRM_SIZE+1] = {};
	unsigned char * r_prm[NPRM_SIZE] = {rp0,rp1,rp2,rp3,rp4,rp5};
	unsigned char ri = 0;
	unsigned char rfi = 0;
	unsigned char rpi = 0;
	unsigned char i = 0;

	toUpperStr(rt->rb);

	// Take limited command word, store in TERMINAL
	while ((rt->rb[ri]) && (rt->rb[ri] != SPACE)) {
		rt->cmd[ri] = rt->rb[ri];
		if (ri == CMD_SIZE) { break; }
		ri++;
	}
	rt->cmd[ri] = '\0';



	// Take any flags and parameters, storing flags immediately in TERMINAL
	while ((rt->rb[ri])) {
		if ((rfi < NFLG_SIZE) && (rt->rb[ri]=='-')) {
			rt->flgs[rfi] = rt->rb[ri+1];
			ri++;		// Extra ri increment to get to flag
			rfi++;		// Increment flag index
		} else if ((rpi < NPRM_SIZE) && (rt->rb[ri] != SPACE)) {
			while ((rt->rb[ri]) && (i < PRM_SIZE) && (rt->rb[ri] != SPACE)) {
				*(r_prm[rpi]+i) = rt->rb[ri];
				ri++;	// Increment buffer index
				i++;	// Increment internal param index
			}
			*(r_prm[rpi]+i) = '\0';	// Null cap internal param
			i = 0;		// Reset internal param index
			rpi++;		// Increment param index
		}
		if (rt->rb[ri]) { ri++; }	// Only increment when still valid
	}
	rt->flgs[rfi] = '\0';	// Null cap contents of r_flg

	// Convert each parameter to hexadecimal value, place in TERMINAL
	i = 0;	// Re-use i
	while (*r_prm[i] && i < NPRM_SIZE) {
		rt->prms[i] = (unsigned char)strtoul((char*)r_prm[i],(char**)NULL,16);
		i++;
	}
	rt->prms[i] = '\0';

}

#endif // UART_TERM