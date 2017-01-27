#include <stdio.h>
#include <avr/pgmspace.h>

#include "../dkfile_uart.h"
#include "hotbutton.h"


char *dkaction_button1( char *line )
{
    PORTE.OUT = ~(1<<1);                    // turn on only LED1
                                            
    dkuart_stuff_string( 0, dk_hotbuttons[1] );     // stuff the hotbutton string defined for this button into uart buffer
    

    return line;
}

