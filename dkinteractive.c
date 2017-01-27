#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "dkterm.h"
#include "dkvt100.h"
#include "dkstring.h"
#include "dktables.h"


//--------------------------------------------------------------------
void dkinteractive(void)
{
	char line[MAX_LINE_LENGTH+1];
	char token[MAX_TOKEN_LENGTH+1];
	char *lptr;
	char * (*actionhandler)(char *str) = NULL;


	printf_P( PSTR("\n\n") );
	dkterm_clearscreen();
	printf_P( PSTR("INTERACTIVE GLITCHER %1.2f   %s %s\n"), _GLITCHER_VERSION_NUM/100.0, __DATE__, __TIME__ );

		
	while( 1 )                       	// loop forever in interpreter
	{
		if( io_ctrl_char == DKKEY_CTRLC )
		{
            io_ctrl_char = 0;                   // unset ctrl-c signal
            _flushuntil( DKKEY_CTRLC );         // flush everything up to and including ctrl-c from input buffer   
            printf_P( PSTR("\n") );
	    }


	    printf_P( PSTR("> ") );         // display prompt
	    
    	dkgets( line );

		printf_P( PSTR("\n") );

		if( !strlen( line ) )
			continue;

		lptr = dkgettoken( line, token );
		if( !strlen(token) )
			continue;

        if( !strlen( token) )
            continue;
            
        actionhandler = dkfindaction( token );
        if( actionhandler )
        {
            lptr = actionhandler( lptr );
        }
        else
        {
            printf_P( PSTR("Unknown action \"%s\".\n"), token );
        }

	}
} 

