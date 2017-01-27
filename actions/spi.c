#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#include "../dkstring.h"
#include "../dkterm.h"


// SPI master transmit len bytes from data
int spi_m_tx( uint8_t *data, int len );


char *dkaction_spi( char *line )
{
    char *token=NULL;
    uint8_t data[255];                      // data to be transmitted
    int len;
    int i;
        
        
    line = dkgettoken( line, token );       // read first token from the line
    for( len=0; strlen(token); line=dkgettoken(line, token), len++ )   // loop through all tokens on the line
        data[ len ] = strtol( token, NULL, 16 );    // convert token to hex byte

    if( !len )
        return line;                        // length == 0, return

    printf_P( PSTR("SPI TX:") );            // print data to be transmitted
    for( i=0; i<len; i++ )
    {
        if( !(i%16) )
            printf_P( PSTR("\n") );         // print newline every 16 bytes
        printf_P( PSTR(" %02X"), data[i] );
    }
    printf_P( PSTR("\nTalking to SPI slave... ") );


    len = spi_m_tx( data, len );            // SPI TX len bytes

    
    printf_P( PSTR("\r                                \r") );
    printf_P( PSTR("SPI RX:") );            // print received data
    
    for( i=0; i<len; i++ )
    {
        if( !(i%16) )
            printf_P( PSTR("\n") );         // print newline every 16 bytes
        printf_P( PSTR(" %02X"), data[i] );
    }
    printf_P( PSTR("\n") );


    return line;
}


// SPI master transmit len bytes from data
int spi_m_tx( uint8_t *data, int len )
{
    int i;


    PORTF.DIRSET = (1<<4)|(1<<5)|(1<<7);            // configure pins SS, MOSI, SCK = output
    PORTF.DIRCLR = (1<<6);                          // MISO = input

    SPIF.INTCTRL = 0;                               // no SPIF interrupt
//    SPIF.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (1<<SPI_MODE_gp) | (1<<SPI_PRESCALER_gp);   // SPI_MODE 01 = rising edge=setup, falling edge=sample

//DORD=0 = msb first	~SPI_DORD_bm
//mode 0 - CPOL=0 CPHA=0
    SPIF.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | (3<<SPI_MODE_gp) | (1<<SPI_PRESCALER_gp);   // SPI_MODE 01 = rising edge=setup, falling edge=sample

    
    PORTF.OUTCLR = (1<<4);                          // set SS low    
    
    if( SPIF.STATUS & SPI_IF_bm )
        i = SPIF.DATA;                              // clear SPIF flag

    for( i=0; i<len; i++ )
    {
        SPIF.DATA = data[i];
        
        _putch('.');
        while( !( SPIF.STATUS & SPI_IF_bm ) )
            if( io_ctrl_char == DKKEY_CTRLC )
                break;
//            ;                                       // wait for SPIF flag to be set
        
        data[i] = SPIF.DATA;
    }

    PORTF.OUTSET = (1<<4);                          // release SS
    

    return i;
}
