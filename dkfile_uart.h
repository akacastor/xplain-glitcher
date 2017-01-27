//--------------------------------------------------------------------
#ifndef _dkfile_uart_h
#define _dkfile_uart_h
//--------------------------------------------------------------------

#include "dkfile.h"

//--------------------------------------------------------------------

#define DK_PARITY_NONE		0
#define DK_PARITY_ODD		1
#define DK_PARITY_EVEN		2

#define DK_FLOWCTRL_CTSRTS	(1<<0)
#define DK_FLOWCTRL_XONXOFF	(1<<1)



#define DKUART_FLAG_TERMINAL	(1<<0)	// this device is active for terminal io


struct _dk_uart
{
	uint8_t portnum;
	uint32_t speed;
	uint8_t parity;
	uint8_t data;
	uint8_t stop;
	uint8_t flowctrl;					// what type of flow control to use - bitmapped
	uint8_t _flag;

	uint16_t rx_buflen;					// total size of rx buffer (in bytes)
	uint16_t rx_buflimit;				// limit in number of bytes at which to assert flow control
	uint8_t *rx_buffer;					// circular buffer for received data (note: if this is changed from 128 byte len, code requires changes elsewhere also)
	volatile uint16_t rx_wr_ptr, rx_rd_ptr;		// will be pointed at dkuart0_rx_buffer, dkuart0_rx_wr_ptr, dkuart0_rx_rd_ptr, etc
};



// isr's should already be installed for all available COM ports
// just need to set TXEN/RXEN/RXCIEx?
// for max flexibility, the isr's should be same as for console io
//  - can detach console and open from basic app
//  - can stop telly
//  - dktelly should go through generic dkuart character interface
// use pointer for rxbuffer on each com port? or hardcoded buffer lengths?

extern int dkuart_open( DKFILE *stream );
extern int dkuart_close( void *deviceinfo );
extern int dkuart_read_byte(void *deviceinfo);
extern int dkuart_write_byte(void *deviceinfo, int c);
extern int dkuart_read_bytes(void *deviceinfo, int n, void *ptr);
extern int dkuart_write_bytes(void *deviceinfo, int n, void *ptr );
extern int dkuart_seek(void *deviceinfo, off_t offset, int whence);
extern fpos_t dkuart_pos(void *deviceinfo);
extern int dkuart_eof(void *deviceinfo);
extern int dkuart_error(void *deviceinfo);

extern int dkuart_flowctrl(void *deviceinfo, int status );

extern void dkuart_stuff_byte( int idx, uint8_t data );
extern int dkuart_stuff_string( int idx, char *data );


extern struct _dkdeviceio uartdeviceio;
extern struct _dk_uart dkuart[];


//--------------------------------------------------------------------
#endif	// !_dkfile_uart_h
//--------------------------------------------------------------------
