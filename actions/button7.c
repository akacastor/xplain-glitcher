#include <stdio.h>
#include <avr/pgmspace.h>

#include "../dkfile_uart.h"
#include "hotbutton.h"


char *dkaction_button7( char *line )
{
    PORTE.OUT = (uint8_t)~(1<<7);                    // turn on only LED7
                                            
    dkuart_stuff_string( 0, dk_hotbuttons[7] );     // stuff the hotbutton string defined for this button into uart buffer
    

    return line;
}

