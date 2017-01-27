#include <stdio.h>
#include <avr/pgmspace.h>

#include "../dkfile_uart.h"
#include "hotbutton.h"


char *dkaction_button2( char *line )
{
    PORTE.OUT = ~(1<<2);                    // turn on only LED2
                                            
    dkuart_stuff_string( 0, dk_hotbuttons[2] );     // stuff the hotbutton string defined for this button into uart buffer
    

    return line;
}

