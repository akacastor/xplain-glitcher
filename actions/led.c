#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>

#include "../dkstring.h"


char *dkaction_led( char *line )
{
    char *token=NULL;
    int led;
    int status;
    
    
    line = dkgettoken( line, token );   // read first token from the line
	if( !strlen(token) )
	    return line;

    led = strtol( token, NULL, 0 );


    line = dkgettoken( line, token );   // read second token from the line
	if( strlen(token) )
        status = strtol( token, NULL, 0 );  // a status was specified
    else
        status = (PORTE.OUT & (1<<led));    // no status specified in command line - toggle led

    
    printf_P( PSTR("LED %d %s\n"), led, status?"on":"off" );

    if( status )
        PORTE.OUTCLR = 1<<led;
    else
        PORTE.OUTSET = 1<<led;        


    return line;
}

