#include <stdio.h>
#include <avr/pgmspace.h>

#include "../dkfile_uart.h"
#include "hotbutton.h"


char *dkaction_button6( char *line )
{
    PORTE.OUT = ~(1<<6);                    // turn on only LED6
                                            
    dkuart_stuff_string( 0, dk_hotbuttons[6] );     // stuff the hotbutton string defined for this button into uart buffer
    

    return line;
}

