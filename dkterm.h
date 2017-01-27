//--------------------------------------------------------------------
#ifndef _dkterm_h
#define _dkterm_h
//--------------------------------------------------------------------
#include <stdio.h>
#include "dkfile.h"

//--------------------------------------------------------------------
// bitmasks for dkterm_interfaces
#define DKTERM_VT100				1

//--------------------------------------------------------------------

struct _dkscreen
{
	uint8_t row;
	uint8_t col;

	uint8_t width;
	uint8_t height;

	uint8_t scrolltop;
	uint8_t scrollbot;
};

//--------------------------------------------------------------------

struct _dkterminal
{
	char name[8];
	
	int (*put)( char c, DKFILE *devfile );
	int (*get)( DKFILE *devfile );
	int (*kbhit)( DKFILE *devfile );
	int (*gotoxy)( int x, int y, DKFILE *devfile );
	int (*clearscreen)( DKFILE *devfile);	
};


struct _dkterm_interfaces
{
	DKFILE *devfile;
	struct _dkterminal *terminal;
	struct _dkterm_interfaces *next;
};

extern struct _dkscreen dkscreen;

//extern uint8_t rx_buffer[128];							// circular buffer for received data
//extern uint16_t rx_buflen;
//extern volatile uint8_t rx_wr_ptr, rx_rd_ptr;			// pointers into rx_buffer

extern volatile uint8_t io_ctrl_char;					// control character if received (chars 0x01-0x1F)

// uart_str is used to setup stdin/stdout
extern FILE stdiostream;

extern struct _dkterm_interfaces *dkterm_interfaces;	// list of active terminal interfaces

//--------------------------------------------------------------------

extern int _kbhit(void);
extern int _getch(void);
extern void _putch( char c );

extern void _flushuntil( char c );          // flush input buffer up to and including c

//--------------------------------------------------------------------
extern int dkterm_init(void);
extern int dkterm_put( char c, FILE *unused );
extern int dkterm_get( FILE *unused );
extern int dkterm_gotoxy( int x, int y );
extern int dkterm_clearscreen( void );
//--------------------------------------------------------------------
#endif	// !_term_h
//--------------------------------------------------------------------
