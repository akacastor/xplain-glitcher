//--------------------------------------------------------------------
#ifndef _dktables_h
#define _dktables_h
//--------------------------------------------------------------------
#include <avr/pgmspace.h>


// this routine is in dkactiontable.S
extern char * (*dkfindaction( char *str ))(char *str );


// this routine is in dktables.S (used by dkfindstatement/dkfindfunction)
extern void (*dksearchtable( char *str, PGM_P ))(char *str );


extern uint8_t * dkactiontable;

//--------------------------------------------------------------------
#endif	// !_dktables_h
//--------------------------------------------------------------------

