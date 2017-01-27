#include <stdio.h>
#include <avr/pgmspace.h>

#include "../dkfile_uart.h"
#include "hotbutton.h"


char *dkaction_button5( char *line )
{
    PORTE.OUT = ~(1<<5);                    // turn on only LED5
                                            
    dkuart_stuff_string( 0, dk_hotbuttons[5] );     // stuff the hotbutton string defined for this button into uart buffer
    

    return line;
}

