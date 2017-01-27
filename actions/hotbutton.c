#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "../dkfile_uart.h"
#include "../dkterm.h"
#include "../dkstring.h"
#include "hotbutton.h"


char dk_hotbuttons[DK_NUM_HOTBUTTONS][DK_HOTBUTTON_MAXLEN]=
{ 
    "HELP\r", 
    "PEEK ", 
    "POKE ", 
    "LED ", 
    "BUTTON", 
    "F6",           // "BEEP\r", 
    "F7", 
    "F8"
};


char *dkaction_hotbutton( char *line )
{
    char *token=NULL;
    uint8_t button;
    int i,n;
    
    
    line = dkgettoken( line, token );   // read first token from the line
	if( !strlen(token) )
	    return line;
    
    if( token[0] == '?' )               // HOTBUTTON ?  = list hotbutton settings
    {
        for( i=0; i<DK_NUM_HOTBUTTONS; i++ )
        {
            printf_P( PSTR("BUTTON%d = \""), i, dk_hotbuttons[i] );

            for( n=0; dk_hotbuttons[i][n]; n++ )
            {
                if( dk_hotbuttons[i][n] == '\r' || dk_hotbuttons[i][n] == '\n' )
                {
                    _putch( '\\' );          // print escaped newlines
                    _putch( 'n' );  
                }
                else
                    _putch( dk_hotbuttons[i][n] );                
            }
            printf_P( PSTR("\"\n") );
        }

        return line;
    }

    button = strtol( token, NULL, 0 );
    if( button >= DK_NUM_HOTBUTTONS || button < 0 )
        return line;
   
    line = dkgettoken( line, token );   // read second token from the line

    if( token[0] == '\"' && token[strlen(token)-1] == '\"' )
    {
        token[strlen(token)-1] = '\0';  // remove trailing quote from token
        token++;                        // remove leading quote from token
    }

    strncpy( dk_hotbuttons[ button ], dkunescape( token ), DK_HOTBUTTON_MAXLEN);
    dk_hotbuttons[button][DK_HOTBUTTON_MAXLEN-1] = '\0';

    printf_P( PSTR("BUTTON%d = \""), button, dk_hotbuttons[ button ] );

    for( n=0; dk_hotbuttons[button][n]; n++ )
    {
        if( dk_hotbuttons[button][n] == '\r' || dk_hotbuttons[button][n] == '\n' )
        {
            _putch( '\\' );          // print escaped newlines
            _putch( 'n' );  
        }
        else
            _putch( dk_hotbuttons[button][n] );                
    }
    printf_P( PSTR("\"\n") );


    return line;
}

