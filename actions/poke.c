#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>

#include "../dkstring.h"


char *dkaction_poke( char *line )
{
    char *token=NULL;
    unsigned int address;
    unsigned char data;
    int n=0;
    
    
    line = dkgettoken( line, token );   // read first token from the line
	if( !strlen(token) )
	    return line;

    address = strtol( token, NULL, 0 );


    line = dkgettoken( line, token );   // read second token from the line
	while( strlen(token) )
    {
        if( !(n%16) )
        {
            if( n )
                printf_P( PSTR("\n") );
            printf_P( PSTR("%04X="), address+n );
        }

        data = strtol( token, NULL, 0 );
    
        printf_P( PSTR(" %02X"), data );
    
        *( (unsigned char *)(address+n) ) = data;

        line = dkgettoken( line, token );   // read next token from the line
        n++;
    }
    
    printf_P( PSTR("\n") );
    

    return line;
}

