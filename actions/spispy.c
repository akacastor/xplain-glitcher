//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "../dkstring.h"
#include "../dkterm.h"


//////////////////////////////////////////////////////////////////////
// SPIF (Xplained J1) = primary slave   (MOSI)
// SPIC (Xplained J4) = secondary slave (MISO)
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// if SPISPY_BUFLEN is made > 255, head and tail pointers need to be changed from uint8_t to uint16_t
#define SPISPY_BUFLEN   255

volatile uint8_t spydata_mosi[ SPISPY_BUFLEN ];
volatile uint8_t spydata_miso[ SPISPY_BUFLEN ];
volatile uint8_t spy_mosi_head, spy_mosi_tail;       // head and tail pointer for circular buffer
volatile uint8_t spy_miso_head, spy_miso_tail;


//////////////////////////////////////////////////////////////////////
char *dkaction_spispy( char *line )
{
    spy_mosi_head = 0;
    spy_mosi_tail = 0;
    spy_miso_head = 0;
    spy_miso_tail = 0;


// setup SPIF as slave
    SPIF.INTCTRL = 0;                               // no SPIF interrupt (we only need one interrupt, since data is synchronous)
    SPIF.CTRL = SPI_ENABLE_bm | (1<<SPI_MODE_gp);   // SPI_MODE 01 = rising edge=setup, falling edge=sample

// setup SPIC as slave
    SPIC.INTCTRL = (2<<SPI_INTLVL_gp);              // enable SPIC interrupt, level 2
    SPIC.CTRL = SPI_ENABLE_bm | (1<<SPI_MODE_gp);   // SPI_MODE 01 = rising edge=setup, falling edge=sample

    PMIC.CTRL |= PMIC_MEDLVLEX_bm;                  // Enable PMIC interrupt level medium
	sei();                                          // Enable global interrupts (should be already enabled)



    printf_P( PSTR("Waiting for SPI data...\n") );

    while( io_ctrl_char != DKKEY_CTRLC )
    {
// wait for data from spi 1
// wait for data from spi 2 (should be available at same time as spi 1)
        if( (spy_mosi_head == spy_mosi_tail) || (spy_miso_head == spy_miso_tail) )
            continue;                   // data not yet received in mosi and miso buffers, continue to wait
        
                                        // display received pair of bytes on screen
        printf( "%02X %02X\n", spydata_mosi[ spy_mosi_head++ ], spydata_miso[ spy_miso_head++ ] );
    }
    io_ctrl_char = 0;                   // unset ctrl-c signal
    _flushuntil( DKKEY_CTRLC );         // flush everything up to and including ctrl-c from input buffer   


    SPIC.INTCTRL = 0;                   // disable SPIC interrupt


    return line;
}


ISR(SPIC_INT_vect)
{
        // fill circular buffers with received SPI data
    spydata_mosi[ spy_mosi_tail++ ] = SPIF.DATA;
    spydata_miso[ spy_miso_tail++ ] = SPIC.DATA;
    
    if( spy_mosi_tail >= SPISPY_BUFLEN )
        spy_mosi_tail = 0;
    if( spy_miso_tail >= SPISPY_BUFLEN )
        spy_miso_tail = 0;
}
