//--------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include "dkvt100.h"
#include "dkfile.h"
//--------------------------------------------------------------------

struct _dkterminal dkterm_vt100 =
{
	"VT100",
	dkvt100_put,
	dkvt100_get,
	dkvt100_kbhit,
	dkvt100_gotoxy,
	dkvt100_clearscreen
};



//--------------------------------------------------------------------
int dkvt100_put( char c, DKFILE *devfile )
{
	dkfputc( c, devfile );


	return 0;
}
//--------------------------------------------------------------------
int dkvt100_get( DKFILE *devfile )
{
	return dkfgetc( devfile );
}
//--------------------------------------------------------------------
int dkvt100_kbhit( DKFILE *devfile )
{
	return !dkfeof( devfile );
}
//--------------------------------------------------------------------
int dkvt100_gotoxy( int x, int y, DKFILE *devfile )
{
	char tempstr[6];				// temporary string used for itoa
	int i;
	

	dkfputc( 0x1B, devfile );
	dkfputc( '[', devfile );
	itoa( y, tempstr, 10 );
	for( i=0; tempstr[i]; i++ )
		dkfputc( tempstr[i], devfile );
	dkfputc( ';', devfile );
	itoa( x, tempstr, 10 );
	for( i=0; tempstr[i]; i++ )
		dkfputc( tempstr[i], devfile );
	dkfputc( 'f', devfile );


	return 0;
}

//--------------------------------------------------------------------
int dkvt100_clearscreen( DKFILE *devfile)
{
	dkfputc( 0x1B, devfile );			// ANSI clear screen and home cursor
	dkfputc( '[', devfile );
	dkfputc( '2', devfile );
	dkfputc( 'J', devfile );
	

	return 0;
}
//--------------------------------------------------------------------
