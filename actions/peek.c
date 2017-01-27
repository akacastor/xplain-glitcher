#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>

#include "../dkstring.h"


char *dkaction_peek( char *line )
{
    char *token=NULL;
    uint32_t address;
    uint32_t len=1;
    uint32_t i;
    
    
    line = dkgettoken( line, token );   // read first token from the line
	if( !strlen(token) )
	    return line;

    address = strtol( token, NULL, 0 );

    line = dkgettoken( line, token );   // read first token from the line
	if( strlen(token) )
        len = strtol( token, NULL, 0 );
    
    
    for( i=0; i<len; i++ )
    {
        if( !(i%16) )
        {
            if( i )
                printf_P( PSTR("\n" ) );
            printf_P( PSTR("%04X:"), address+i ); 
        }
            // a (uint16_t) cast here supresses a compiler warning due to avr-gcc's 16-bit address space limit
        printf_P( PSTR(" %02X"), *( (unsigned char *)(uint16_t)(address+i) ) );
    }
    printf_P( PSTR("\n") );


    return line;
}

