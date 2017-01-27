#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "../dkterm.h"
#include "../dkstring.h"


char *dkaction_disco( char *line )
{
    char *token=NULL;
    char animation_sequence[]="|/-\\";
    uint8_t i=0;
    
    
    line = dkgettoken( line, token );   // read first token from the line
	if( strlen(token) )
	    srand( strtol( token, NULL, 0 ) );
    
    
    printf_P( PSTR("Party time! (ctrl-c to quit)\n") );
    
    while( io_ctrl_char != DKKEY_CTRLC )    // loop until a ctrl-c is received
    {
        PORTE.OUT = rand();             // set LED0-LED7 to random states
        
        if( !animation_sequence[++i] )
            i=0;
        _putch( animation_sequence[i] );    // display next step in animation
        _putch( '\b' );
        
	    _delay_ms(200);
    }
    io_ctrl_char = 0;                   // unset ctrl-c signal
    _flushuntil( DKKEY_CTRLC );         // flush everything up to and including ctrl-c from input buffer   

    puts( " " );


    return line;
}

