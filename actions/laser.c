#include <stdio.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#include "../dkterm.h"
#include "../hardware.h"
#include "../dkstring.h"



void Delay_ms(int n) {
  while (n--) {
   _delay_ms(1);
 }
}

char *dkaction_laser( char *line )
{
    char *token = NULL;
    int glitch_len = 100;
    int delay_len = 200;

    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        glitch_len = strtoul(token,NULL,0);
        
        line = dkgettoken( line, token ); 
        if( strlen(token) )
        {
            delay_len = strtoul(token,NULL,0);
        }
    }



    printf( "Laser pointer test...\n" );

    while( io_ctrl_char != DKKEY_CTRLC )
    {
        G2981_PORT.DIRSET = (1<<G2981_SW);
        G2981_PORT.OUTSET = (1<<G2981_SW);

        Delay_ms(glitch_len);

        G2981_PORT.OUTCLR = (1<<G2981_SW);
        
        
        Delay_ms(delay_len);
    }


    return line;
}
