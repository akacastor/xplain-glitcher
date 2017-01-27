#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <ctype.h>

#include "../dkfile_uart.h"
#include "../dkterm.h"
#include "../dkstring.h"
#include "hotkey.h"

//char dk_funckeystr[10][16]={ "HELP\n", "PEEK ", "POKE ", "LED ", "BUTTON", "BEEP\n", "F7", "F8", "F9", "F10" };

char dk_hotkeys[DK_NUM_HOTKEYS][DK_HOTKEY_MAXLEN]=
{ 
    "HELP\r", 
    "PEEK ", 
    "POKE ", 
    "LED ", 
    "BUTTON", 
    "BEEP\r", 
    "F7", 
    "F8",
    "F9",
    "F10",
    "F11",
    "F12"
};


char *dkaction_hotkey( char *line )
{
    char *token=NULL;
    uint8_t hotkey;
    int i,n;
    
    
    line = dkgettoken( line, token );   // read first token from the line
	if( !strlen(token) )
	    return line;
    
    if( token[0] == '?' )               // HOTKEY ?  = list hotkey settings
    {
        for( i=0; i<8; i++ )
        {
            printf_P( PSTR("F%d = \""), i+1, dk_hotkeys[i] );

            for( n=0; dk_hotkeys[i][n]; n++ )
            {
                if( dk_hotkeys[i][n] == '\r' || dk_hotkeys[i][n] == '\n' )
                {
                    _putch( '\\' );          // print escaped newlines
                    _putch( 'n' );  
                }
                else
                    _putch( dk_hotkeys[i][n] ); 
            }
            printf_P( PSTR("\"\n") );
        }

        return line;
    }

    if( tolower( token[0] ) == 'f' )
        hotkey = strtol( token+1, NULL, 0 )-1;      // F1=0, F2=1, etc
    else
        hotkey = strtol( token, NULL, 0 );          // read index 0, 1, 2, etc
        
    if( hotkey >= DK_NUM_HOTKEYS || hotkey < 0 )
        return line;

    line = dkgettoken( line, token );   // read second token from the line

    if( token[0] == '\"' && token[strlen(token)-1] == '\"' )
    {
        token[strlen(token)-1] = '\0';  // remove trailing quote from token
        token++;                        // remove leading quote from token
    }

    strncpy( dk_hotkeys[ hotkey ], dkunescape( token ), DK_HOTKEY_MAXLEN);
    dk_hotkeys[ hotkey ] [DK_HOTKEY_MAXLEN-1 ] = '\0';

    printf_P( PSTR("F%d = \""), hotkey+1, dk_hotkeys[ hotkey ] );

    for( n=0; dk_hotkeys[hotkey][n]; n++ )
    {
        if( dk_hotkeys[hotkey][n] == '\r' || dk_hotkeys[hotkey][n] == '\n' )
        {
            _putch( '\\' );          // print escaped newlines
            _putch( 'n' );  
        }
        else
            _putch( dk_hotkeys[hotkey][n] );                
    }
    printf_P( PSTR("\"\n") );


    return line;
}

