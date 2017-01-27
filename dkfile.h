//--------------------------------------------------------------------
#ifndef _dkfile_h
#define _dkfile_h
//--------------------------------------------------------------------

#include <stdio.h>


//--------------------------------------------------------------------

typedef uint16_t fpos_t;
typedef uint32_t off_t;


#define FILENAME_MAX 		255
//#define DKFOPEN_MAX		32

typedef struct _dkfile DKFILE;

// struct _dkdeviceio contains functions for dealing with each low-level device
struct _dkdeviceio
{
	int (*open)(DKFILE *stream);
	int (*close)(void *deviceinfo);

	int (*read_byte)(void *deviceinfo);
	int (*write_byte)(void *deviceinfo, int c);
	int (*read_bytes)(void *deviceinfo, int c, void *ptr );
	int (*write_bytes)(void *deviceinfo, int c, void *ptr );
	int (*seek)(void *deviceinfo, off_t offset, int whence);
	fpos_t (*pos)(void *deviceinfo);
	
	int (*eof)(void *deviceinfo);
	int (*error)(void *deviceinfo);
};


// struct _dkfile is used as DKFILE (equivalent to FILE from stdio.h)
struct _dkfile
{
	int fd;					// file descriptor (0=stdin/1=stdout/2=stderr)
	int filenum;			// BASIC file number (starting at 1)
	char *filename;			// name of this file
	uint16_t _flag;			// flags for this file (input/output/both/type of file?)
	
	void *info;				// device-specific information structure
	struct _dkdeviceio *io;
	
	
	struct _dkfile *next;	// the next DKFILE in the linked list
};



extern DKFILE *_dkfile_list;	// linked list of open DKFILEs

//--------------------------------------------------------------------

#define DKFILE_FLAG_READ		(1<<0)	// read access
#define DKFILE_FLAG_WRITE		(1<<1)	// write access

#define DKFILE_FLAG_UART		(1<<2)	// UART COM port
#define DKFILE_FLAG_EEPROM		(1<<3)	// file in avr eeprom
#define DKFILE_FLAG_FLASH		(1<<4)	// file in avr flash
#define DKFILE_FLAG_FAT			(1<<5)	// fatfs, file on SD card
#define DKFILE_FLAG_TWI			(1<<6)	// TWI (i2c) device (port+address)
#define DKFILE_FLAG_SPI			(1<<7)	// SPI port
#define DKFILE_FLAG_KBD			(1<<8)	// PS2 keyboard

//#define DKFILE_FLAG_KEYBOARD

//--------------------------------------------------------------------

extern DKFILE *dkfopen(const char *filename, const char *mode);
extern int dkfclose(DKFILE *stream);
extern int dkfseek(DKFILE *stream, off_t offset, int whence);
extern long dkftell(DKFILE *stream);
extern int dkfgetc(DKFILE *stream);
extern int dkfputc(int c, DKFILE *stream);
extern char *dkfgets(char *s, int n, DKFILE *stream);
extern size_t dkfread(void *ptr, size_t size, size_t nitems, DKFILE *stream);
extern size_t dkfwrite(const void *ptr, size_t size, size_t nitems, DKFILE *stream);
extern int dkfeof(DKFILE *stream);
extern int dkferror(DKFILE *stream);


extern DKFILE *dkfilebynum( int filenum );			// return a pointer to DKFILE with BASIC filenum


//--------------------------------------------------------------------
#endif	// !_dkfile_h
//--------------------------------------------------------------------
