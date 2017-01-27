//--------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#include "dkterm.h"
#include "dkstring.h"
#include "dkfile_uart.h"
#include "dkvt100.h"
//--------------------------------------------------------------------

volatile uint8_t io_ctrl_char;						// control character if received (chars 0x01-0x1F)

// uart_str is used to setup stdin/stdout
FILE stdiostream = FDEV_SETUP_STREAM(dkterm_put, dkterm_get, _FDEV_SETUP_RW);


struct _dkscreen dkscreen;

struct _dkterm_interfaces *dkterm_interfaces=NULL;	// list of active terminal interfaces


//--------------------------------------------------------------------
int dkterm_init(void)
{
	struct _dkterm_interfaces *newinterface;


	io_ctrl_char = 0;					// no ctrl char has been received

	stdout = stdin = &stdiostream;		// setup stdout and stdin streams

	dkterm_interfaces = NULL;			// default to no active interfaces (_init functions will setup dkterm_interfaces)

	dkscreen.row = 1;					// set initial values for dkscreen
	dkscreen.col = 1;

	dkscreen.width = 80;				// regular terminal is 80 cols wide
//	dkscreen.width = 38;				// tellymate is 38 cols wide
	dkscreen.height = 25;

	dkscreen.scrolltop = 1;				// define top and bottom of scrolling area (1-25 = entire screen scrolls)
	dkscreen.scrollbot = 25;			// note: using a smaller scrolling area results in slower scrolling
										// note: scrolling is tellymate-only

//#ifdef _DK_USE_UART0
//	printf_P( PSTR("\nInit UART0...     ") );

	newinterface = (struct _dkterm_interfaces *)malloc( sizeof( struct _dkterm_interfaces ) );
	newinterface->next = NULL;		        

	newinterface->devfile = dkfopen( "COM0:115200,N,8,1", "r+" );
	((struct _dk_uart *)newinterface->devfile->info)->_flag |= DKUART_FLAG_TERMINAL;
	newinterface->terminal = &dkterm_vt100;		// indicate that VT100 terminal mode is active

	dkterm_interfaces = newinterface;           // link new _dkterm_interfaces item into list
	
//	printf_P( PSTR("\rInit UART...      OK\n") );
//#endif	// _DK_USE_UART0


	return 1;
}

//--------------------------------------------------------------------

int dkterm_put( char c, FILE *unused )
{
	struct _dkterm_interfaces *ifaces;


    while( io_ctrl_char == DKKEY_PAUSE )
	    ;                           // pause received, wait until that signal removed
	

	for( ifaces=dkterm_interfaces; ifaces; ifaces=ifaces->next )
		ifaces->terminal->put( c, ifaces->devfile );

    if( dkterm_interfaces && ((struct _dk_uart *)dkterm_interfaces->devfile->info)->_flag & DKUART_FLAG_TERMINAL )
    {
	    switch( c )
	    {
	    	case '\n':
	    		dkscreen.row++;
	    		if( dkscreen.row > dkscreen.height )
	    			dkscreen.row = dkscreen.height;
	    		dkterm_put( '\r', NULL );			// send CR with LF
	    		break;
	    
	    	case '\r':
	    		dkscreen.col = 1;
	    		break;
	    	
	    	case '\b':
	    		dkscreen.col--;
	    		if( dkscreen.col < 1 )
	    			dkscreen.col = 1;
	    		break;
	    	
	    	default:
	    		dkscreen.col++;
	    		break;
	    }
        
	    if( dkscreen.col > dkscreen.width )			// print newline after col80
	    {
	    	dkterm_put( '\n', NULL );
	    	return 0;
	    }
    }
	
	return 0;
	
/*
		if( dkterm_interfaces & dkterm_TELLY )
		{
		if( dkscreen.row < dkscreen.scrollbot )
		{
			dkscreen.row++;
			telly_put('\n');
		}
		else
		{									// note: handling the tellymate scrolling here isn't very clean - should be in dktelly.c
			if( dkscreen.scrolltop > 1 )
			{
				if( topregion )
					free( topregion );
				topregion = (char *)malloc( (dkscreen.scrolltop-1) * dkscreen.width );
				if( !topregion )		// malloc error!
					return 0;

				telly_put(0x1B);			// turn cursor off
				telly_put('f');

				telly_goto( 1, 1 );
				for( i = 0 ; i+1 < dkscreen.scrolltop; i++ )
				{
					for( n = 0; n < dkscreen.width; n++ )
					{			// save line from screen
						topregion[ (i*dkscreen.width) + n ] = telly_charhere();
						telly_put(0x1B);				// advance cursor
						telly_put('C');					// if we overwrite with a space now, there is a lot of flicker in non-scroll region
					}
					telly_put('\n');
				}
				telly_goto( dkscreen.scrollbot, 1 );
			}


			if( dkscreen.height-dkscreen.scrollbot )
			{
				botregion = (char *)malloc( (dkscreen.height-dkscreen.scrollbot) * dkscreen.width );
				if( !botregion )		// malloc error!
				{
					if( topregion )
						free( topregion );
					return 0;
				}

				telly_put(0x1B);			// turn cursor off
				telly_put('f');
//				_delay_ms(10);				// a short delay is required here for cursor to turn off


				telly_goto( dkscreen.scrollbot, 1 );
				for( i = 0 ; i+dkscreen.scrollbot+1 <= dkscreen.height; i++ )
				{
					telly_put('\n');
					for( n = 0; n < dkscreen.width; n++ )
					{			// save line from screen
						botregion[ (i*dkscreen.width) + n ] = telly_charhere();
						telly_put(0x1B);				// advance cursor
						telly_put('C');					// if we overwrite with a space now, there is a lot of flicker in non-scroll region
					}
				}
// scroll screen

			

// reprint lines saved from screen
				telly_goto( dkscreen.scrollbot+1, 1 );
				for( i = 0 ; i+dkscreen.scrollbot+1 <= dkscreen.height; i++ )
				{
				telly_put(0x1B);			// clear line
				telly_put('l');
					telly_put('\n');
					for( n = 0; n < dkscreen.width; n++ )
					{		
						if( botregion[ (i*dkscreen.width) + n ] )
							telly_put( botregion[ (i*dkscreen.width) + n ] );
						else
							telly_put( ' ' );
					}
				}

				free( botregion );

				telly_goto( dkscreen.scrollbot-1, 1 );
				telly_put(0x1B);			// clear line
				telly_put('l');

//				telly_put(0x1B);			// turn cursor on
//				telly_put('e');
			}
			else
				telly_put( '\n' );

			if( dkscreen.scrolltop > 1 )
			{
// reprint lines saved from screen
				telly_goto( 1, 1 );
				for( i = 0 ; i+1 < dkscreen.scrolltop; i++ )
				{
				telly_put(0x1B);			// clear line
				telly_put('l');

					for( n = 0; n < dkscreen.width; n++ )
					{			// save line from screen
						if( topregion[ (i*dkscreen.width) + n ] )
							telly_put( topregion[ (i*dkscreen.width) + n ] );
						else
							telly_put( ' ' );
					}
					telly_put('\n');
				}

				free( topregion );

				telly_goto( dkscreen.scrollbot-1, 1 );

//				telly_put(0x1B);			// turn cursor on
//				telly_put('e');
			}
				telly_put(0x1B);			// turn cursor on
				telly_put('e');
		}
		}//		if( dkterm_interfaces & dkterm_TELLY )
*/

}

//--------------------------------------------------------------------
int dkterm_get( FILE *unused )
{
	struct _dkterm_interfaces *ifaces;
	

	while( 1 )								// loop until ctrl-c received or byte received
	{
		for( ifaces=dkterm_interfaces; ifaces; ifaces=ifaces->next )
		{
			if( ifaces->terminal->kbhit( ifaces->devfile ) )
				return ifaces->terminal->get( ifaces->devfile );
		}
	}
	
	return -1;
}

//--------------------------------------------------------------------
int _kbhit(void)
{
	struct _dkterm_interfaces *ifaces;


	for( ifaces=dkterm_interfaces; ifaces; ifaces=ifaces->next )
	{		
		if( ifaces->terminal->kbhit( ifaces->devfile ) )
			return 1;
	}

	
	return 0;
}


//--------------------------------------------------------------------
int _getch(void)
{
	return dkterm_get( NULL );
}


//--------------------------------------------------------------------
void _putch( char c )
{
	dkterm_put( c, NULL );

	return;
}

//--------------------------------------------------------------------

//- standard dkio interface functions for gotoxy, getfromscreen, clearscreen, etc
int dkterm_gotoxy( int x, int y )
{
	struct _dkterm_interfaces *ifaces;
	

	for( ifaces=dkterm_interfaces; ifaces; ifaces=ifaces->next )
		ifaces->terminal->gotoxy( x, y, ifaces->devfile );

	dkscreen.col=x;
	dkscreen.row=y;

		
	return 0;
}

//--------------------------------------------------------------------
int dkterm_clearscreen( void )
{
	struct _dkterm_interfaces *ifaces;
	

	for( ifaces=dkterm_interfaces; ifaces; ifaces=ifaces->next )
		ifaces->terminal->clearscreen( ifaces->devfile );
		
	return 0;
}
//--------------------------------------------------------------------
void _flushuntil( char c )          // flush input buffer up to and including c
{
    while( _kbhit() )           // flush everything in buffer up to and including c
    {
        if( _getch() == c )
            break;
    }
}
//--------------------------------------------------------------------
