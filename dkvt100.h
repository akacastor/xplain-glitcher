//--------------------------------------------------------------------
#ifndef _dkvt100_h
#define _dkvt100_h
//--------------------------------------------------------------------
#include "dkterm.h"

//--------------------------------------------------------------------
extern int dkvt100_put( char c, DKFILE *devfile );
extern int dkvt100_get( DKFILE *devfile );
extern int dkvt100_kbhit( DKFILE *devfile );
extern int dkvt100_gotoxy( int x, int y, DKFILE *devfile );
extern int dkvt100_clearscreen( DKFILE *devfile);


extern struct _dkterminal dkterm_vt100;


//--------------------------------------------------------------------
#endif	// !_dkvt100_h
//--------------------------------------------------------------------
