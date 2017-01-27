//--------------------------------------------------------------------
#ifndef _dkstring_h
#define _dkstring_h
//--------------------------------------------------------------------
//#include "dkexpression.h"


// http://www.computer-engineering.org/ps2keyboard/scancodes2.html

// 0xAA is value returned by keyboard after reset and BAT
#define DKKEY_BATOK		0xAA


#define DKKEY_NUMLOCK	0x77
#define DKKEY_SCROLL	0x7E

#define DKKEY_PAUSE		(0x80+0x7A)
#define DKKEY_PRTSCR	(0x80+0x7B)
#define DKKEY_SYSRQ		(0x80+0x7C)
//#define DKKEY_BREAK		(0x80+0x7E)
// BREAK is same as ctrl-C
#define DKKEY_BREAK		(0x03)

#define DKKEY_NUM_9		(0x80+0x19)
#define DKKEY_NUM_8		(0x80+0x18)
#define DKKEY_NUM_7		(0x80+0x17)
#define DKKEY_NUM_6		(0x80+0x16)
#define DKKEY_NUM_5		(0x80+0x15)
#define DKKEY_NUM_4		(0x80+0x14)
#define DKKEY_NUM_3		(0x80+0x13)
#define DKKEY_NUM_2		(0x80+0x12)
#define DKKEY_NUM_1		(0x80+0x11)
#define DKKEY_NUM_0		(0x80+0x10)
#define DKKEY_NUM_PER	(0x80+0x0E)

#define DKKEY_DEL		(0x80+0x6E)
#define DKKEY_INS		(0x80+0x70)
#define DKKEY_END		(0x80+0x71)
#define DKKEY_DOWN		(0x80+0x72)
#define DKKEY_PGDN		(0x80+0x73)
#define DKKEY_LEFT		(0x80+0x74)
#define DKKEY_RIGHT		(0x80+0x76)
#define DKKEY_HOME		(0x80+0x77)
#define DKKEY_UP		(0x80+0x78)
#define DKKEY_PGUP		(0x80+0x79)


#define DKKEY_F1		(0x80+0x01)
#define DKKEY_F2		(0x80+0x02)
#define DKKEY_F3		(0x80+0x03)
#define DKKEY_F4		(0x80+0x04)
#define DKKEY_F5		(0x80+0x05)
#define DKKEY_F6		(0x80+0x06)
#define DKKEY_F7		(0x80+0x07)
#define DKKEY_F8		(0x80+0x08)
#define DKKEY_F9		(0x80+0x09)
#define DKKEY_F10		(0x80+0x0A)
#define DKKEY_F11		(0x80+0x0B)
#define DKKEY_F12		(0x80+0x0C)


#define DKKEY_APPS		(0x80+0x2F)


// ACPI scan codes
#define DKKEY_POWER		(0x80+0x37)
#define DKKEY_SLEEP		(0x80+0x3F)
#define DKKEY_WAKE		(0x80+0x5E)

// Windows Multimedia Scan Codes:  (from http://www.computer-engineering.org/ps2keyboard/scancodes2.html)
#define DKKEY_NEXTTRACK		(0x80+0x4D)
#define DKKEY_PREVTRACK		(0x80+0x15)		///
#define DKKEY_STOP			(0x80+0x3B)
#define DKKEY_PLAYPAUSE		(0x80+0x34)
#define DKKEY_MOUTE			(0x80+0x23)
#define DKKEY_VOLUP			(0x80+0x32)
#define DKKEY_VOLDOWN		(0x80+0x21)
#define DKKEY_MEDIA			(0x80+0x50)
#define DKKEY_EMAIL			(0x80+0x48)
#define DKKEY_CALCULATOR	(0x80+0x2B)
#define DKKEY_MYCOMPUTER	(0x80+0x40)
#define DKKEY_WWWSEARCH		(0x80+0x10)		/////
#define DKKEY_WWWHOME		(0x80+0x3A)
#define DKKEY_WWWBACK		(0x80+0x38)
#define DKKEY_WWWFORWARD	(0x80+0x30)
#define DKKEY_WWWSTOP		(0x80+0x28)
#define DKKEY_WWWREFRESH	(0x80+0x20)
#define DKKEY_WWWFAVORITES	(0x80+0x18)		/////

#define DKKEY_CTRLC         (0x03)

//--------------------------------------------------------------------
// KEY definitions (for F1-F10)

extern char dk_funckeystr[10][16];



// note: always allocate one extra byte (for NULL terminator)
#define MAX_LINE_LENGTH		512

// note: MAX_TOKEN_LENGTH limits the length of "" strings! (TODO: pass dynamic max string length to dkgettoken)
#define MAX_TOKEN_LENGTH	48



//--------------------------------------------------------------------

void dkgets( char *s );
// dkpregets - like gets, but the string s can be pre-filled
void dkpregets( char *s );
void dkpregetsi( char *s, int i );

// dkvt102input() - return 'c' scancode value converted from VT102 escape sequence
uint8_t dkvt102input(void);

char *dkgettoken( char *expstring, char *t );

//extern char *dkgetnameflags( char *expstring, uint16_t *flags );

double dkstrtonum( char *s );

//extern dkexpression *dkgetnextexpression( char * line );
//extern dkexpression *dkgetfirstexpression( char * line );

char *dkdequote( char *t );		// remove " " from ends of string

char *dkunescape( char *t );


//--------------------------------------------------------------------
#endif	// !_dkstring_h
//--------------------------------------------------------------------

