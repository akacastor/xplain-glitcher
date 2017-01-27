//--------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "dkstring.h"
#include "dkterm.h"
#include "actions/hotkey.h"


//--------------------------------------------------------------------
void dkgets( char *s )
{
	s[0] = '\0';
	dkpregets( s );

	return;
}

//--------------------------------------------------------------------
void dkpregets( char *s )
{
	int i=0;

	dkpregetsi( s, i );

	return;
}
//--------------------------------------------------------------------
void dkpregetsi( char *s, int i )
{
	uint16_t l=0;						// length of s
	uint16_t t;
	uint16_t c;
	uint16_t n;
	static uint8_t insertmode=0;		// TODO: make insertmode global?


	l = strlen(s);
	if( l )
	{
		printf_P( PSTR("%s"), s );
		for( i=l; i; i-- )
			_putch('\b');
	}
	
	while(1)
	{
		c=_getch();

		if( c == 0x1B )		// esc received
			c = dkvt102input();

		switch(c)
		{
			case '\n':
				continue;

			case '\r':
				return;

			case '\b':
				if(i)
				{
					if( i < l )
					{
						for( t=i; t<l; t++ )					// shift right part of s over one position (to left)
							s[t-1] = s[t];
						i--;
						l--;									// length decrements by one
						s[l] = '\0';
						_putch('\b');
						for( t=i; t<l; t++ )					// display right part of s
							_putch( s[t] );
						_putch(' ');
						for( t=i; t<=l; t++ )
							_putch( '\b' );
					}
					else
					{
						i--;
						l--;
						s[i] = '\0';

						_putch('\b');
						_putch(' ');
						_putch('\b');
					}
				}
				continue;

			case '\x03':				// ctrl-c
				s[0] = '\0';
				return;

			case 0x1B:					// ESC
				s[0] = '\0';

				for( ; i<l ; i++ )
					_putch( ' ' );	// move a space to the right - loop until at end of string

				for( i=0; i<l; i++ )				// backspace over entire line
				{
					_putch('\b');
					_putch(' ');
					_putch('\b');
				}
				i = 0;
				l = 0;
				s[0]='\0';

				continue;


//			case DKKEY_DOWN:			// down arrow
			case DKKEY_RIGHT:			// right arrow
				if( i<l )
				{
					_putch( s[i] );	// move a space to the right (re-print current char)
					i++;
				}
				continue;

//			case DKKEY_UP:			// up arrow
			case DKKEY_LEFT:			// left arrow
				if( i )
				{
					i--;
					_putch('\b');	// move a space to the left
				}
				continue;

			case DKKEY_HOME:			// ESC [ 1 ~   home
				for( ; i ; i-- )
					_putch('\b');	// move a space to the left - loop until at beginning of string
				continue;

			case DKKEY_END:			// ESC O F   end
				for( ; i<l ; i++ )
					_putch( s[i] );	// move a space to the right - loop until at end of string
				continue;

			case DKKEY_INS:			// ESC [ 2 ~   INS
				insertmode = !insertmode;	// toggle insert mode
				continue;

			case DKKEY_DEL:			// ESC [ 3 ~   DEL
				t = i;			// save current s idx
				for( ; i<l ; i++ )
				{
					s[i] = s[i+1];
					_putch( s[i] );
				}
				if( t<l )
				{
					l--;		// adjust length for character deleted
					_putch(' ');
				}
				for( ; i>t ; i-- )
					_putch('\b');
				continue;

			case DKKEY_PGUP:			// ESC [ 5 ~   PGUP
			case DKKEY_PGDN:			// ESC [ 6 ~   PGDN
				continue;
		}

		if( c >= DKKEY_F1 && c <= DKKEY_F10 )
		{								// handle function keys (F1-F10 'keys' shortcuts)
			n = strlen( dk_hotkeys[c-DKKEY_F1] );
			if( !n )
				continue;

			if( dk_hotkeys[c-DKKEY_F1][n-1] == '\r' )
				n--;

			strncpy( s+i, dk_hotkeys[c-DKKEY_F1], n );

			for( t=0 ; t<n ; t++ )
				_putch( s[i++] );

			if( i > l )
				l = i;
			s[l] = '\0';			// null-terminate string

			if( dk_hotkeys[c-DKKEY_F1][n] == '\r' )
				return;

			continue;
		}


//		if( c > 0x7E || c < 0x20 )
//			continue;					// ignore any characters >0x7E (DEL or anything with high bit set - extended chars)

        

		if( c < 0x20 )
			continue;					// ignore any characters <0x20
//        {
//            _putch( 0x2F );           // debug display
//            _putch( c+0x30 ); 
//            _putch( 0x2F );
//        }

		if( insertmode && i<l )
		{								// insert mode - insert character into string
			_putch(c);
			for( t=l; t>=i && t; t-- )					// shift right part of s over one position
				s[t] = s[t-1];
			if( l < MAX_LINE_LENGTH )				// only increment length if string hasn't reached MAX_LINE_LENGTH
				l++;									// length increments by one (insert mode)
			for( t=i+1; t<l; t++ )					// display right part of s
				_putch( s[t] );
			for( t=i; t<l; t++ )
				_putch( '\b' );
			s[i++] = c;
		}
		else
		{								// not insert mode - overwrite (or, at end of string so nothing to adjust after this char)
			s[i++] = c;
			if( i > l )
			{
				if( l < MAX_LINE_LENGTH )
					l++;				// increment line length
				else
				{
					i = l;				// max line length reached, don't increment length or i
					_putch( '\b' );
				}
			}
		}
		s[l] = '\0';
		_putch(c);
	}
}



//--------------------------------------------------------------------
uint8_t dkvt102input(void)
{
	uint8_t c=0x1B;
	uint8_t modifiers=0;


	if( !_kbhit() )
		_delay_ms(11);                  // 11 ms is just enough time for next byte @ 9600bps
	if( _kbhit() )
	{									// get key following ESC
		c = _getch();
		
		if( c == 0x1B )                 // received ESC ESC - this is vt100 telling us alt-key is pressed with arrow keys
		{
		    modifiers |= 1;             // alt-key pressed

			if( !_kbhit() )
				_delay_ms(11);
			if( !_kbhit() )
			    return c;
			
			c = _getch();
		}
		
		if( c == '[' || c == 'O' )
		{								// received ESC [ (or ESC O for end key)
			if( !_kbhit() )
				_delay_ms(11);
			if( _kbhit() )
			{
				c = _getch();		// get byte after ESC [

				switch( c )
				{
					case 'A':			// up arrow
						c = DKKEY_UP;
						break;

					case 'D':			// left arrow
						c = DKKEY_LEFT;
						break;

					case 'B':			// down arrow
						c = DKKEY_DOWN;
						break;

					case 'C':			// right arrow
						c = DKKEY_RIGHT;
						break;

					case '1':			// ESC [ 1 ~   home
						if( !_kbhit() )
						_delay_ms(11);
						if( _kbhit() )
							_getch();	// get trailing ~
						c = DKKEY_HOME;
						break;

					case 'F':			// ESC O F   end
						c = DKKEY_END;
						break;

					case '4':			// ESC [ 4 ~   END
						if( !_kbhit() )
							_delay_ms(11);
						if( _kbhit() )
							_getch();	// get trailing ~
						c = DKKEY_END;
						break;

					case '2':			// ESC [ 2 ~   INS
						if( !_kbhit() )
							_delay_ms(11);
						if( _kbhit() )
							_getch();	// get trailing ~
						c = DKKEY_INS;
						break;

					case '3':			// ESC [ 3 ~   DEL
						if( !_kbhit() )
							_delay_ms(11);
						if( _kbhit() )
							_getch();	// get trailing ~
						c = DKKEY_DEL;
						break;

					case '5':			// ESC [ 5 ~   PGUP
						if( !_kbhit() )
							_delay_ms(11);
						if( _kbhit() )
							_getch();	// get trailing ~
						c = DKKEY_PGUP;
						break;

					case '6':			// ESC [ 6 ~   PGDN
						if( !_kbhit() )
							_delay_ms(11);
						if( _kbhit() )
							_getch();	// get trailing ~
						c = DKKEY_PGDN;
						break;
				    
				    case 'P':   // ESC [ P  F1
				    case 'Q':
				    case 'R':
				    case 'S':
				    case 'T':
				    case 'U':
				    case 'V':
				    case 'W':
				    case 'X':
				    case 'Y':
				    case 'Z':
				    case '[':
				        c = DKKEY_F1 + c-'P';
				        break;
				}

				return c;
			}
		}
	}				

	return c;
}


//--------------------------------------------------------------------
//get next token  - note: t must be already allocated
char *dkgettoken( char *expstring, char *t )
{
	uint16_t tidx=0;
	uint8_t inquote=0;

	
	t[0] = '\0';						// start token string (t) as empty string


	while( (*expstring) && tidx<MAX_TOKEN_LENGTH )		// loop until end of string is reached
	{
		if( ((*expstring)>='A'&&(*expstring)<='Z') || ((*expstring)>='a'&&(*expstring)<='z') || 
				((*expstring)=='.') || ((*expstring)=='_') || ((*expstring)>='0'&&(*expstring)<='9') )
		{
			t[ tidx++ ] = (*expstring);
			t[ tidx ] = '\0';
		}
		else if( (*expstring)=='\"' )					// quote
		{
			if( inquote )
			{											// got closing quote
				t[ tidx++ ] = (*expstring++);
				t[ tidx ] ='\0';
				return expstring;
			}
			inquote=1;									// got opening quote
			t[ tidx++ ] = (*expstring);
			t[ tidx ] ='\0';
		}
		else if( inquote )
		{
			t[ tidx++ ] = (*expstring);
			t[ tidx ] = '\0';
		}
		else if( ((*expstring)==' ') || ((*expstring)=='\t') )
		{												// whitespace, not inside quotes
			if( tidx )
				return expstring;
		}
		else if( (*expstring)=='\'' )
		{
			return expstring+strlen(expstring);
		}
		else											// symbol
		{
			if( tidx )
				return expstring;
			t[ tidx ] = (*expstring++);
			if( t[tidx] == '<' && expstring[0] == '=' )
			{											// got <=
				expstring++;
				t[ ++tidx ] = '=';
			}
			else if( t[tidx] == '>' && expstring[0] == '=' )
			{											// got >=
				expstring++;
				t[ ++tidx ] = '=';
			}
			else if( t[tidx] == '<' && expstring[0] == '>' )
			{											// got <>
				expstring++;
				t[ ++tidx ] = '>';
			}
			t[ ++tidx ] = '\0';
			return expstring;
		}
		expstring++;
	}

	return expstring;
}

/*
//--------------------------------------------------------------------
// char *dkgetnameflags( char *expstring, uint16_t *flags )
//  flags is updated by this function
// expstring - string to read name from ('expression string')
// flags - pointer to flags word - must point to a valid word
// returns: pointer to expstring
char *dkgetnameflags( char *expstring, uint16_t *flags )
{
	char t[ MAX_TOKEN_LENGTH+1 ];		// space for token parsed by dkgettoken()
	char *strptr=NULL;

	(*flags) = 0;

	while( expstring[0] )				// loop until end of string (or end of type specifiers)
	{
		strptr = dkgettoken( expstring, t );
		switch( t[0] )
		{
			case '(':					// parenthesis - array or function
				(*flags) |= DKNAME_PAREN;
				return expstring;

			case '$':					// string
				if( (*flags) )			
					return expstring;	// if there's already a type specifier, ignore this character
				(*flags) |= DKNAME_STRING;
				expstring = strptr;
				break;
	
			case '%':					// integer
				if( (*flags) )			
					return expstring;	// if there's already a type specifier, ignore this character
				(*flags) |= DKNAME_INTEGER;
				expstring = strptr;
				break;
	
			case '!':					// single precision
				if( (*flags) )			
					return expstring;	// if there's already a type specifier, ignore this character
				(*flags) |= DKNAME_FLOATSINGLE;
				expstring = strptr;
				break;
	
		 	case '#':					// double precision
				if( (*flags) )			
					return expstring;	// if there's already a type specifier, ignore this character
				(*flags) |= DKNAME_FLOATDOUBLE;
				expstring = strptr;
				break;

		 	case '@':					// byte (DK-BASIC custom)
				if( (*flags) )			
					return expstring;	// if there's already a type specifier, ignore this character
				(*flags) |= DKNAME_BYTE;
				expstring = strptr;
				break;

			default:					// token is not a type specifier
				return expstring;
		}
	}

	return expstring;
}
*/

//--------------------------------------------------------------------

double dkstrtonum( char *s )
{
	uint16_t intval;
	uint8_t i;


	if( s[0] == '&' )
	{
		if( s[1]=='h' || s[1]=='H' )
			sscanf( s+2, "%x", &intval );
		else if( s[1]=='o' || s[1]=='O' )
			sscanf( s+2, "%o", &intval );
		else if( s[1]=='b' || s[1]=='B' )
		{
			intval = 0;
			for( i=2; s[i]=='0' || s[i]=='1'; i++ )			
			{
				intval<<=1;
				if( s[i]=='1' )
					intval |= 1;
			}			
		}
		else
			sscanf( s+1, "%o", &intval );

		return intval;
	}
	else
	{
		return atof( s );
	}
}

/*
//--------------------------------------------------------------------
// read next expression for a function - expects to see , then expression or ) if no more expressions
// returns NULL when ) reached
dkexpression *dkgetnextexpression( char * line )
{
	dkexpression *e;
	char t[ MAX_TOKEN_LENGTH+1 ];


	line = dkgettoken( line, t );
	expstr = line;

	if( t[0] == ')' )
		return NULL;					// reached end of function parameters

	if( t[0] != ',' )
		return NULL;					// error: syntax error (missing closing parenthesis)

	e = expression( line );				// process expression @line
	if( !e )	
		return NULL;					// error: missing operand


	return e;
}


//--------------------------------------------------------------------
// read one expression for a function - expects to see opening parenthesis (
dkexpression *dkgetfirstexpression( char * line )
{
	dkexpression *e;
	char t[ MAX_TOKEN_LENGTH+1 ];


	line = dkgettoken( line, t );		// get token - (
	if( t[0] != '(' )
		return NULL;					// error: syntax error (missing closing parenthesis)

	e = expression( line );				// process expression @line
	if( !e )	
		return NULL;					// error: missing operand


	return e;
}
*/

//--------------------------------------------------------------------
// remove leading and trailing quotes if they exist
char *dkdequote( char *t )
{
	if( strlen(t) > 1 && t[ 0 ] =='\"' )
	{
	    if( t[ strlen(t)-1 ] == '\"' )
    		t[ strlen(t)-1 ] = '\0';		// remove trailing "
		return t+1;						// skip over leading "
	}
	return t;
}

//--------------------------------------------------------------------

char *dkunescape( char *t )
{
    int i;
    
    if( !t[0] )
        return t;                       // null string
    
    for( i=0; t[i+1]; i++ )
    {
        if( t[i] == '\\' )
        {
            switch( t[i+1] )
            {
                case '\\':
                    i++;
                    strcpy( t+i, t+i+1 );
                    break;
                
                case 'n':
                case 'r':
                    t[i] = '\r';
                    i++;
                    strcpy( t+i, t+i+1 );
                    break;
            }            
        }
    }
    
    return t;    
}

//--------------------------------------------------------------------
