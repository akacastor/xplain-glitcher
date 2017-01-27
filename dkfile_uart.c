//--------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

//#include "dkbasic.h"
#include "dkfile_uart.h"
#include "dkstring.h"
#include "dkterm.h"
//#include "dkevents.h"


//--------------------------------------------------------------------

struct _dkdeviceio uartdeviceio =
{
	dkuart_open, 
	dkuart_close, 
	dkuart_read_byte, 
	dkuart_write_byte, 
	dkuart_read_bytes, 
	dkuart_write_bytes, 
	dkuart_seek, 
	dkuart_pos,
	dkuart_eof,
	dkuart_error
};


//--------------------------------------------------------------------

struct _dk_uart dkuart[2];      // dkuart[0] == USARTC0, dkuart[1] == USARTF0


//--------------------------------------------------------------------
// parse filename for port #, speed, etc
//OPEN "COM[n]:[speed][,parity][,data] [,stop][,RS][,CS[n]][,DS[n]][,CD[n]][,LF] [,PE]" AS [#]filenum [LEN=number]
int dkuart_open( DKFILE *stream )
{
	char t[MAX_TOKEN_LENGTH];					// used for token parsing with dkgettoken()
	char *filename;
	struct _dk_uart uartconfig;
    uint16_t new_BSEL;

												// fill in deviceinfo with defaults then update it with values from filenaem
	uartconfig.portnum=-1;
	uartconfig.speed=57600;
	uartconfig.parity=DK_PARITY_NONE;
	uartconfig.data=8;
	uartconfig.stop=1;
	uartconfig.flowctrl = DK_FLOWCTRL_CTSRTS;
	uartconfig._flag=0;

	uartconfig.rx_buflen = 512;
	uartconfig.rx_buflimit = 448;

	
	filename = stream->filename;

	filename += 3;								// skip past "COM"

	filename = dkgettoken( filename, t );		// get [n] as token

	if( isdigit( *t ) )
		uartconfig.portnum = atoi( t );
	else
		return 1;								// error in filename (no port #)


	filename = dkgettoken( filename, t );		// get : as token
		
	if( (*t) != ':' )
		return 1;								// error in filename! (or filename done?)

		
	filename = dkgettoken( filename, t );		// get [speed] as token
	if( t[0] != ',' && t[0] )
	{											// got a speed token
		uartconfig.speed = atol( t );			// use (*t) speed token
		
		filename = dkgettoken( filename, t );	// get , as token
		
		if( t[0] && ( t[0] != ',' ) )
			return 1;							// error in filename (or filename done)
	}


	filename = dkgettoken( filename, t );		// get [parity] as token
	if( t[0] != ',' && t[0] )
	{											// got a parity token
		switch( tolower((*t)) )					// use (*t) speed token
		{
			case 'n':
				uartconfig.parity = DK_PARITY_NONE;
				break;
				
			case 'o':
				uartconfig.parity = DK_PARITY_ODD;
				break;
				
			case 'e':
				uartconfig.parity = DK_PARITY_EVEN;
				break;
		}
		
		filename = dkgettoken( filename, t );	// get , as token
		
		if( (*t) && ( (*t) != ',' ) )
			return 1;							// error in filename (or filename done)
	}

		
	filename = dkgettoken( filename, t );		// get [data] as token
	if( t[0] != ',' && t[0] )
	{											// got a data token
												
		uartconfig.data = atoi( t );			// use (*t) data token
		
		filename = dkgettoken( filename, t );	// get , as token
		
		if( (*t) && ( (*t) != ',' ) )
			return 1;							// error in filename (or filename done)
	}

		
	filename = dkgettoken( filename, t );		// get [stop] as token
	if( t[0] != ',' && t[0] )
	{											// got a stop token
												
		uartconfig.stop = atoi( t );			// use (*t) stop token
		
		filename = dkgettoken( filename, t );	// get , as token
		
		if( (*t) && ( (*t) != ',' ) )
			return 1;							// error in filename (or filename done)
	}



	uartconfig.rx_buffer = (uint8_t *)malloc( uartconfig.rx_buflen );		// allocate space for rx buffer

	
	switch( uartconfig.portnum )
	{
		case 0:								// init UART0
			dkuart[0].portnum = uartconfig.portnum;
			dkuart[0].speed = uartconfig.speed;
			dkuart[0].parity = uartconfig.parity;
			dkuart[0].data = uartconfig.data;
			dkuart[0].stop = uartconfig.stop;
			dkuart[0].flowctrl = uartconfig.flowctrl;
			dkuart[0]._flag = uartconfig._flag;

			dkuart[0].rx_buflen = uartconfig.rx_buflen;
			dkuart[0].rx_buflimit = uartconfig.rx_buflimit;

			if( dkuart[0].rx_buffer )
				free( dkuart[0].rx_buffer );					// free previous rx_buffer if it existed
			dkuart[0].rx_buffer = uartconfig.rx_buffer;
			
			dkuart[0].rx_wr_ptr=0;
			dkuart[0].rx_rd_ptr=0;
			
			stream->info = &dkuart[0];

    
//    PORTCFG_VPCTRLA = 0x23;             // map PORTC to virtual port 1, PORTD to virtual port 0

// set PORTC output for txd
//    VPORT1_OUT |= 0x08; 
//    VPORT1_DIR |= 0x08;        
            PORTC.OUT |= (1<<3);        // set PC3 (TXD) to 1
            PORTC.DIR |= (1<<3);        // set PC3 (TXD) as output


//			UBRR0 = ( F_CPU / (double)(16 * uartconfig.speed) + 0.5 ) - 1;
//			UCSR0A = (0<<U2X0);

            
            new_BSEL = 16 * ( (float)(2000000.0/uartconfig.speed) - 1);


//            USARTC0_BAUDCTRLA = 0x1C;     // 57.6 kbps @ 32 MHz
//            USARTC0_BAUDCTRLB = 0xC2;     // BSEL = 540, BSCALE=-4

            USARTC0_BAUDCTRLA = new_BSEL & 0xFF;
            USARTC0_BAUDCTRLB = 0xC0 | ( (new_BSEL>>8) & 0x0F);


//    USARTC0_BAUDCTRLA = 0x13;       // 57.6 kbps @ 2 MHz
//    USARTC0_BAUDCTRLB = 0xC0;       // BSEL = 19, BSCALE=-4

    
            USARTC0_CTRLA = (1<<USART_RXCINTLVL_gp);    // RXC interrupt level 1

            	// Enable PMIC interrupt level low
	        PMIC.CTRL |= PMIC_LOLVLEX_bm;
	// Enable global interrupts
	        sei();


            USARTC0_CTRLC = 0x03;       // 8 bit data
            USARTC0_CTRLB = USART_TXEN_bm | USART_RXEN_bm;


/*	TODO: additional serial port configuration
- character size: 5,6,7,8,9
UCSRxB / UCSRxC

UCSRxC:
- parity N/E/O
- stop bits 1/2
*/
//			UCSR0B = (1<<TXEN0) | (1<<RXEN0) | (1<<RXCIE0);

			if( dkuart[0].flowctrl & DK_FLOWCTRL_CTSRTS )
			{
//				PORTE &= (~(1<<2));
//				DDRE |= (1<<2);
			}

			break;
			

		case 1:								// init UART1		TODO: if port is already open, close it first (in case of rx interrupt during setup)
			dkuart[1].portnum = uartconfig.portnum;
			dkuart[1].speed = uartconfig.speed;
			dkuart[1].parity = uartconfig.parity;
			dkuart[1].data = uartconfig.data;
			dkuart[1].stop = uartconfig.stop;
			dkuart[1].flowctrl = uartconfig.flowctrl;
			dkuart[1]._flag = uartconfig._flag;

			dkuart[1].rx_buflen = uartconfig.rx_buflen;
			dkuart[1].rx_buflimit = uartconfig.rx_buflimit;

			if( dkuart[1].rx_buffer )
				free( dkuart[1].rx_buffer );					// free previous rx_buffer if it existed
			dkuart[1].rx_buffer = uartconfig.rx_buffer;
			
			dkuart[1].rx_wr_ptr=0;
			dkuart[1].rx_rd_ptr=0;
			
			stream->info = &dkuart[1];

    
//    PORTCFG_VPCTRLA = 0x23;             // map PORTC to virtual port 1, PORTD to virtual port 0

// set PORTC output for txd
//    VPORT1_OUT |= 0x08; 
//    VPORT1_DIR |= 0x08;        
            PORTF.OUT |= (1<<3);        // set PF3 (TXD) to 1
            PORTF.DIR |= (1<<3);        // set PF3 (TXD) as output


//			UBRR0 = ( F_CPU / (double)(16 * uartconfig.speed) + 0.5 ) - 1;
//			UCSR0A = (0<<U2X0);

            
            new_BSEL = 16 * ( (float)(2000000.0/uartconfig.speed) - 1);


//            USARTC0_BAUDCTRLA = 0x1C;     // 57.6 kbps @ 32 MHz
//            USARTC0_BAUDCTRLB = 0xC2;     // BSEL = 540, BSCALE=-4

            USARTF0_BAUDCTRLA = new_BSEL & 0xFF;
            USARTF0_BAUDCTRLB = 0xC0 | ( (new_BSEL>>8) & 0x0F);


//    USARTC0_BAUDCTRLA = 0x13;       // 57.6 kbps @ 2 MHz
//    USARTC0_BAUDCTRLB = 0xC0;       // BSEL = 19, BSCALE=-4

    
            USARTF0_CTRLA = (1<<USART_RXCINTLVL_gp);    // RXC interrupt level 1

            	// Enable PMIC interrupt level low
	        PMIC.CTRL |= PMIC_LOLVLEX_bm;
	// Enable global interrupts
	        sei();


            USARTF0_CTRLC = 0x03;       // 8 bit data
            USARTF0_CTRLB = USART_TXEN_bm | USART_RXEN_bm;


/*	TODO: additional serial port configuration
- character size: 5,6,7,8,9
UCSRxB / UCSRxC

UCSRxC:
- parity N/E/O
- stop bits 1/2
*/
//			UCSR0B = (1<<TXEN0) | (1<<RXEN0) | (1<<RXCIE0);

			if( dkuart[1].flowctrl & DK_FLOWCTRL_CTSRTS )
			{
//				PORTE &= (~(1<<2));
//				DDRE |= (1<<2);
			}

			break;
	}
	
	dkuart_flowctrl( stream->info, 1 );			// set flow control = ready for data

	sei();
	
	return 0;
}

//--------------------------------------------------------------------
int dkuart_close( void *deviceinfo )
{
//stop UART - disable RX interrupt
// destroy rx buffer?


	dkuart_flowctrl( deviceinfo, 0 );			// set flow control = not ready for data

	switch( ((struct _dk_uart *)deviceinfo)->portnum )
	{
		case 0:								// stop UART0
//			UCSR0B = (0<<TXEN0) | (0<<RXEN0) | (0<<RXCIE0);
            USARTC0_CTRLB = 0;
			break;

		case 1:								// stop UART1
//			UCSR1B = (0<<TXEN1) | (0<<RXEN1) | (0<<RXCIE1);
            USARTF0_CTRLB = 0;
			break;
	}
	
	if( ((struct _dk_uart *)deviceinfo)->rx_buffer )
	{
        free( ((struct _dk_uart *)deviceinfo)->rx_buffer );
		((struct _dk_uart *)deviceinfo)->rx_buffer = NULL;
	}


	return 0;
}

//--------------------------------------------------------------------
int dkuart_read_byte(void *deviceinfo)
{
	int c;


	if( ((struct _dk_uart *)deviceinfo)->rx_wr_ptr == ((struct _dk_uart *)deviceinfo)->rx_rd_ptr )
		dkuart_flowctrl( deviceinfo, 1 );			// set flow control = ready for data
	else if( ((struct _dk_uart *)deviceinfo)->rx_wr_ptr > ((struct _dk_uart *)deviceinfo)->rx_rd_ptr )
	{
		if( ( ((struct _dk_uart *)deviceinfo)->rx_wr_ptr - ((struct _dk_uart *)deviceinfo)->rx_rd_ptr ) < ((struct _dk_uart *)deviceinfo)->rx_buflimit )
			dkuart_flowctrl( deviceinfo, 1 );			// set flow control = ready for data
	}
	else
	{
		if( ((struct _dk_uart *)deviceinfo)->rx_buflen - ( ((struct _dk_uart *)deviceinfo)->rx_rd_ptr - ((struct _dk_uart *)deviceinfo)->rx_wr_ptr ) < ((struct _dk_uart *)deviceinfo)->rx_buflimit )
			dkuart_flowctrl( deviceinfo, 1 );			// set flow control = ready for data
	}

	
	while( ((struct _dk_uart *)deviceinfo)->rx_wr_ptr == ((struct _dk_uart *)deviceinfo)->rx_rd_ptr ) 
	{							// wait until character(s) in buffer
		if( io_ctrl_char==3)
			return -1;
	}

	c = (int)((struct _dk_uart *)deviceinfo)->rx_buffer[ ((struct _dk_uart *)deviceinfo)->rx_rd_ptr ];
	( ((struct _dk_uart *)deviceinfo)->rx_rd_ptr )++;

	if( ( ((struct _dk_uart *)deviceinfo)->rx_rd_ptr ) >= ((struct _dk_uart *)deviceinfo)->rx_buflen )
		( ((struct _dk_uart *)deviceinfo)->rx_rd_ptr ) -= ((struct _dk_uart *)deviceinfo)->rx_buflen;


	return c;
}
//--------------------------------------------------------------------
int dkuart_write_byte(void *deviceinfo, int c)
{
	switch( ((struct _dk_uart *)deviceinfo)->portnum )
	{
		case 0:								// send byte out UART0
//			while( ! (UCSR0A & (1<<UDRE0)) )
//				;
//			UDR0 = c;

            while( !(USARTC0_STATUS & USART_DREIF_bm) )
                ;
            USARTC0_DATA = c;

			break;

		case 1:								// send byte out UART1
//			while( ! (UCSR1A & (1<<UDRE1)) )
//				;
//			UDR1 = c;
            while( !(USARTF0_STATUS & USART_DREIF_bm) )
                ;
            USARTF0_DATA = c;

			break;
	}			

	return c;
}
//--------------------------------------------------------------------

int dkuart_read_bytes(void *deviceinfo, int n, void *ptr)
{
	int i;


	for( i=0; i<n; i++ )
		((uint8_t *)ptr)[i] = dkuart_read_byte(deviceinfo);


	return i;
}
//--------------------------------------------------------------------
int dkuart_write_bytes(void *deviceinfo, int n, void *ptr )
{
	int i;


	for( i=0; i<n; i++ )
		dkuart_write_byte( deviceinfo, ((uint8_t *)ptr)[i] );


	return i;
}
//--------------------------------------------------------------------

int dkuart_seek(void *deviceinfo, off_t offset, int whence)
{
	return 0;
}
//--------------------------------------------------------------------
fpos_t dkuart_pos(void *deviceinfo)
{		// for COM ports, pos() returns # of bytes waiting in rx buffer
//	return (((* ((struct _dk_uart *)deviceinfo)->rx_wr_ptr ) - (* ((struct _dk_uart *)deviceinfo)->rx_rd_ptr ) )) %((struct _dk_uart *)deviceinfo)->rx_buflen;
	if( ((struct _dk_uart *)deviceinfo)->rx_wr_ptr == ((struct _dk_uart *)deviceinfo)->rx_rd_ptr )
		return 0;
	else if( ((struct _dk_uart *)deviceinfo)->rx_wr_ptr > ((struct _dk_uart *)deviceinfo)->rx_rd_ptr )
		return ((struct _dk_uart *)deviceinfo)->rx_wr_ptr - ((struct _dk_uart *)deviceinfo)->rx_rd_ptr;
	else
		return ((struct _dk_uart *)deviceinfo)->rx_buflen - ( ((struct _dk_uart *)deviceinfo)->rx_rd_ptr - ((struct _dk_uart *)deviceinfo)->rx_wr_ptr );
}
//--------------------------------------------------------------------
int dkuart_eof(void *deviceinfo)
{
	if( ((struct _dk_uart *)deviceinfo)->rx_wr_ptr != ((struct _dk_uart *)deviceinfo)->rx_rd_ptr ) 
		return 0;

	return -1;
}
//--------------------------------------------------------------------
int dkuart_error(void *deviceinfo)
{
	return 0;
}

//--------------------------------------------------------------------
// 0 = stop, 1 = start
int dkuart_flowctrl( void *deviceinfo, int status )
{
	switch( status )
	{
		case 0:						// stop flow
			if( ((struct _dk_uart *)deviceinfo)->flowctrl & DK_FLOWCTRL_CTSRTS )
			{
				switch( ((struct _dk_uart *)deviceinfo)->portnum )
				{
					case 0:
//						PORTE |= (1<<2);
						break;
						
					case 1:
//						PORTD |= (1<<4);
						break;
				}
			}
			break;
			
		case 1:						// start flow
			if( ((struct _dk_uart *)deviceinfo)->flowctrl & DK_FLOWCTRL_CTSRTS )
			{
				switch( ((struct _dk_uart *)deviceinfo)->portnum )
				{
					case 0:
//						PORTE &= (~(1<<2));
						break;
						
					case 1:
//						PORTD &= (~(1<<4));
						break;
				}
			}
			break;			
	}
				
				
	return 0;
}
//--------------------------------------------------------------------
// this ISR is just slightly too long for data @ 115.2k @ 3.68 MHz
//ISR(USART0_RX_vect) 
ISR(USARTC0_RXC_vect) 
{
	uint8_t data;


//	if( UCSR0A & (_BV(FE0)|_BV(DOR0)) )
//	{
//		data = UDR0;
//		return;
//	}

    if( USARTC0_STATUS & (USART_FERR_bm|USART_BUFOVF_bm|USART_PERR_bm) )
    {
        data = USARTC0_DATA;
        return;
    }

	data = USARTC0_DATA;

	if( dkuart[0].rx_buffer )				// if rx_buffer isn't allocated, the received byte will be lost
	{
		dkuart[0].rx_buffer[ dkuart[0].rx_wr_ptr ] = data;

		dkuart[0].rx_wr_ptr++;
		if( dkuart[0].rx_wr_ptr >= dkuart[0].rx_buflen )
			dkuart[0].rx_wr_ptr -= dkuart[0].rx_buflen;

		if( ((dkuart[0].rx_wr_ptr-dkuart[0].rx_rd_ptr) % dkuart[0].rx_buflen ) >= (dkuart[0].rx_buflimit) )
			dkuart_flowctrl( &dkuart[0], 0 );
	}
	
	if( data == '\x03' )
	{
		if ( dkuart[0]._flag&DKUART_FLAG_TERMINAL )
			io_ctrl_char = data;				// ctrl-c has been received     WHY IS THIS FUCKING ME UP
	}
	else if( data == '\x1A' )
	{
		if ( dkuart[0]._flag&DKUART_FLAG_TERMINAL )
		{
		    if( io_ctrl_char != DKKEY_PAUSE )
			    io_ctrl_char = DKKEY_PAUSE;			// pause (ctrl-z) has been received
            else
                io_ctrl_char = 0;               // pause was received a second time - unpause			    
        }
	}
	
//	dkeventflags |= (1<<0);		// bits set to indicate which COM event fired	
}

//--------------------------------------------------------------------
// this ISR is just slightly too long for data @ 115.2k @ 3.68 MHz
//ISR(USART0_RX_vect) 
ISR(USARTF0_RXC_vect) 
{
	uint8_t data;


//	if( UCSR0A & (_BV(FE0)|_BV(DOR0)) )
//	{
//		data = UDR0;
//		return;
//	}

    if( USARTF0_STATUS & (USART_FERR_bm|USART_BUFOVF_bm|USART_PERR_bm) )
    {
        data = USARTF0_DATA;
        return;
    }

	data = USARTF0_DATA;

	if( dkuart[1].rx_buffer )				// if rx_buffer isn't allocated, the received byte will be lost
	{
		dkuart[1].rx_buffer[ dkuart[1].rx_wr_ptr ] = data;

		dkuart[1].rx_wr_ptr++;
		if( dkuart[1].rx_wr_ptr >= dkuart[1].rx_buflen )
			dkuart[1].rx_wr_ptr -= dkuart[1].rx_buflen;

		if( ((dkuart[1].rx_wr_ptr-dkuart[1].rx_rd_ptr) % dkuart[1].rx_buflen ) >= (dkuart[1].rx_buflimit) )
			dkuart_flowctrl( &dkuart[1], 0 );
	}
	
	if( data == '\x03' )
	{
		if ( dkuart[1]._flag&DKUART_FLAG_TERMINAL )
			io_ctrl_char = data;				// ctrl-c has been received     WHY IS THIS FUCKING ME UP
	}
	else if( data == '\x1A' )
	{
		if ( dkuart[1]._flag&DKUART_FLAG_TERMINAL )
		{
		    if( io_ctrl_char != DKKEY_PAUSE )
			    io_ctrl_char = DKKEY_PAUSE;			// pause (ctrl-z) has been received
            else
                io_ctrl_char = 0;               // pause was received a second time - unpause			    
        }
	}
	
//	dkeventflags |= (1<<0);		// bits set to indicate which COM event fired	
}

//--------------------------------------------------------------------

/*
ISR(USART1_RX_vect) 
{
	uint8_t data;


	if( UCSR1A & _BV(FE1) )
	{
		data = UDR1;
		return;
	}
	if( UCSR1A & _BV(DOR1) )
	{
		data = UDR1;
		return;	
	}

	data = UDR1;

	if( data == '\x03' && (dkuart[1]._flag&DKUART_FLAG_TERMINAL) )
		io_ctrl_char = data;				// ctrl-c has been received

	if( dkuart[1].rx_buffer )				// if rx_buffer isn't allocated, the received byte will be lost
	{
		dkuart[1].rx_buffer[ dkuart[1].rx_wr_ptr ] = data;

		dkuart[1].rx_wr_ptr++;
		if( dkuart[1].rx_wr_ptr >= dkuart[1].rx_buflen )
			dkuart[1].rx_wr_ptr -= dkuart[1].rx_buflen;

		if( ((dkuart[1].rx_wr_ptr-dkuart[1].rx_rd_ptr) % (dkuart[1].rx_buflen+1) ) >= (dkuart[1].rx_buflimit) )
			dkuart_flowctrl( &dkuart[1], 0 );
	}

	dkeventflags |= (1<<1);		// bits set to indicate which COM event fired	
}
*/
//--------------------------------------------------------------------

int dkuart_stuff_string( int idx, char *data )
{
    int n;
        
    for( n=0; data[n]; n++ )
        dkuart_stuff_byte( idx, data[n] );
        
    return n;
}

//--------------------------------------------------------------------

void dkuart_stuff_byte( int idx, uint8_t data )
{
	if( dkuart[idx].rx_buffer )				// if rx_buffer isn't allocated, the received byte will be lost
	{
		dkuart[idx].rx_buffer[ dkuart[idx].rx_wr_ptr ] = data;

		dkuart[idx].rx_wr_ptr++;
		if( dkuart[idx].rx_wr_ptr >= dkuart[idx].rx_buflen )
			dkuart[idx].rx_wr_ptr -= dkuart[idx].rx_buflen;

		if( ((dkuart[idx].rx_wr_ptr-dkuart[idx].rx_rd_ptr) % dkuart[idx].rx_buflen ) >= (dkuart[idx].rx_buflimit) )
			dkuart_flowctrl( &dkuart[idx], 0 );
	}
	
	return;
}

//--------------------------------------------------------------------
