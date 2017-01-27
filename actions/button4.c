#include <stdio.h>
#include <avr/pgmspace.h>

#include "../dkfile_uart.h"
#include "hotbutton.h"


char *dkaction_button4( char *line )
{
    PORTE.OUT = ~(1<<4);                    // turn on only LED4
                                            
    dkuart_stuff_string( 0, dk_hotbuttons[4] );     // stuff the hotbutton string defined for this button into uart buffer
    

    return line;
}

