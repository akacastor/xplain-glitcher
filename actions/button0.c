#include <stdio.h>
#include <avr/pgmspace.h>

#include "../dkfile_uart.h"
#include "hotbutton.h"



// TODO: button should be debounced to avoid multiple triggers caused by a single button press

char *dkaction_button0( char *line )
{
    PORTE.OUT = ~(1<<0);                    // turn on only LED0
                                            
    dkuart_stuff_string( 0, dk_hotbuttons[0] );     // stuff the hotbutton string defined for this button into uart buffer
    

    return line;
}

