//////////////////////////////////////////////////////////////////////
// software for Xmega-A1 Xplained board (blue PCB)
// Chris Gerlinsky, 2010-2016
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "hardware.h"
#include "dkterm.h"
#include "dkvt100.h"
#include "dkstring.h"
#include "dkinteractive.h"
#include "dkbuttons.h"
#include "dkebi.h"
#include "dkclock.h"


//////////////////////////////////////////////////////////////////////

int main( void )
{
    uint8_t led_pattern[] = {0xFE, 0xFD, 0xFB, 0xF7, 0xFF, 0xFF, 0x7F, 0xBF, 0xDF, 0xEF, 0xFF, 0xFF, 0xFE};
    int i;
    
    
    dk_clock_init();                    // initialize clock (32 MHz RC oscillator)    

//	_delay_ms(2000);                    // wait 2 seconds for USB to connect

    PORTE.DIR = 0xFF;                   // set PORTE as output (LEDs)
    
    for( i=0; i < sizeof(led_pattern); i++ )
    {
        PORTE.OUT = led_pattern[i];     // set LEDs to pattern
        _delay_ms( 50 );
    }
    

    dk_ebi_init();                      // initialize the EBI memory
    dkterm_init();                      // initialize dkterm functions (inits dkfile_uart)
//    dkbuttons_init();                   // initialize buttons (PD0-PD6,PR0-PR1) and interrupts



    G2981_PORT.OUTCLR = (1<<G2981_SW);
    G2981_PORT.DIRSET = (1<<G2981_SW);


    GVCC_PORT.OUTSET = (1<<GVCC_EN);            // set ENABLE high (4619 disabled)
    GVCC_PORT.DIRSET = (1<<GVCC_EN);

    GVCC_PORT.OUTCLR = (1<<GVCC_SW);            // clear A and B (select X0/Y0 on 4619, or deactivate ULN2003)
    GVCC_PORT.DIRSET = (1<<GVCC_SW);

    TRST_PORT.OUTCLR = (1<<TRST_BIT);           // PF0 = target reset
    TRST_PORT.DIRSET = (1<<TRST_BIT);              

    TISP_PORT.OUTCLR = (1<<TISP_BIT);           // target ISP = low
    TISP_PORT.DIRSET = (1<<TISP_BIT);              



    while( 1 )
        dkinteractive();                // run the interactive function


    return 0;
}

//////////////////////////////////////////////////////////////////////

