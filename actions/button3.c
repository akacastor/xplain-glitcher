#include <stdio.h>
#include <avr/pgmspace.h>

#include "../dkfile_uart.h"
#include "hotbutton.h"


char *dkaction_button3( char *line )
{
    PORTE.OUT = ~(1<<3);                    // turn on only LED3
                                            
    dkuart_stuff_string( 0, dk_hotbuttons[3] );     // stuff the hotbutton string defined for this button into uart buffer
    

    return line;
}

