//--------------------------------------------------------------------
/*
 - this is implementing some of what should be in stdio.h
*/
//--------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "dkfile.h"
#include "dkfile_uart.h"
//#include "dkfile_twi.h"
//#include "dkfile_spi.h"
//#include "dkfile_eeprom.h"
//#include "dkfile_flash.h"
//#include "dkfile_ps2kbd.h"
//#include "dkbasic.h"
#include "dkterm.h"

//--------------------------------------------------------------------


DKFILE *_dkfile_list=NULL;		// linked list of open DKFILEs


//--------------------------------------------------------------------
// return a pointer to DKFILE with BASIC filenum
DKFILE *dkfilebynum( int filenum )
{
	DKFILE *stream;
	
								
	stream = _dkfile_list;				// point to start of DKFILEs list
	if( !stream )
		return NULL;					// no files in open list

	while( stream->next )
	{
		if( stream->filenum == filenum )
			break;
		stream = stream->next;	// follow list to end
	}
	
	if( stream->filenum != filenum )
		return NULL;
		

	return stream;
}


//--------------------------------------------------------------------

DKFILE *dkfopen(const char *filename, const char *mode)
{
	DKFILE *file_list;
	DKFILE *stream;
	int i=0;
	char devicestr[4];
	
							// TODO: split this into str_to_comconfig ?
								
	stream = (DKFILE *)malloc( sizeof(DKFILE) );
	if( !stream )
		return NULL;					// memory error!
		
									// initialize new FILE struct	
	stream->next = NULL;
	stream->fd = i;						// fd = index into open file chain of this stream
	stream->filenum = 0;
	stream->filename = (char *)malloc( strlen(filename)+1 );
	if( !stream->filename )
		return NULL;					// memory error!
	strcpy( stream->filename, filename );
	stream->_flag = 0;
	stream->io = NULL;
	
	
	while( (*mode) )
	{
		switch( tolower( (*mode) ) )
		{
			case 'r':
				stream->_flag |= DKFILE_FLAG_READ;
				break;
				
			case 'w':
				stream->_flag |= DKFILE_FLAG_WRITE;
				break;
				
			case 'a':
				stream->_flag |= DKFILE_FLAG_READ;
				stream->_flag |= DKFILE_FLAG_WRITE;
														// start at end of file
				break;
			
			case '+':									// TODO: handle r+ / w+ modes properly
				stream->_flag |= DKFILE_FLAG_READ;
				stream->_flag |= DKFILE_FLAG_WRITE;
				break;


			case 't':
			case 'b':
				break;
		}
		mode++;
	}
	
	
// TODO: parse filename to determine type (UART/EEPROM/FLASH/FAT)
// look for a : anywhere in the string?


	if( !strchr( filename, ':' ) )
	{									// no : found - no device name, use default path		
	}
	else
	{
		strncpy( devicestr, filename, 3 );
		devicestr[3] = '\0';					// add null terminator to 3-character device name
		strlwr( devicestr );					// convert device name to lowercase for comparison

		if( !strcmp_P( devicestr, PSTR("com") ) )
			stream->_flag |= DKFILE_FLAG_UART;		
		else if( (!strcmp_P( devicestr, PSTR("i2c") )) || (!strcmp_P( devicestr, PSTR("twi") )) )
			stream->_flag |= DKFILE_FLAG_TWI;
		else if( !strcmp_P( devicestr, PSTR("spi") ) )
			stream->_flag |= DKFILE_FLAG_SPI;
		else if( !strcmp_P( devicestr, PSTR("eep") ) )
			stream->_flag |= DKFILE_FLAG_EEPROM;
		else if( !strcmp_P( devicestr, PSTR("fla") ) )
			stream->_flag |= DKFILE_FLAG_FLASH;
		else if( !strcmp_P( devicestr, PSTR("kbd") ) )
			stream->_flag |= DKFILE_FLAG_KBD;
	}



// select deviceio interface to use for file depending on type (UART/EEPROM/FLASH/FAT)
	if( stream->_flag & DKFILE_FLAG_UART )				// handle as UART file
		stream->io = &uartdeviceio;
//	else if( stream->_flag & DKFILE_FLAG_TWI )			// handle as TWI file
//		stream->io = &twideviceio;
//	else if( stream->_flag & DKFILE_FLAG_SPI )			// handle as SPI file
//		stream->io = &spideviceio;
//	else if( stream->_flag & DKFILE_FLAG_EEPROM )		// handle as EEP file
//		stream->io = &eepromdeviceio;
//	else if( stream->_flag & DKFILE_FLAG_FLASH )		// handle as FLASH file
//		stream->io = &flashdeviceio;
//	else if( stream->_flag & DKFILE_FLAG_KBD )			// handle as PS2 keyboard
//		stream->io = &ps2kbddeviceio;


	if( !stream->io )						// didn't match an io device with this file, can't open it
	{
	    free( stream );
		return NULL;
	}

	
	if( stream->io->open( stream ) )
	{										// error returned from stream->io->open()
		free( stream );
		return NULL;
	}


	file_list = _dkfile_list;				// point to start of DKFILEs list
	if( file_list )
	{
		i++;		
		while( file_list->next )
		{
			file_list = file_list->next;	// follow list to end
			i++;		
		}
			
		file_list->next = stream;			// add new stream to end of _dkfile_list
	}
	else
		_dkfile_list = stream;

	
	return stream;
}


//--------------------------------------------------------------------

int dkfclose(DKFILE *stream)
{
	DKFILE *fileptr;
	
	
	stream->io->close( stream->info );	// close stream

	fileptr = _dkfile_list;				// point to start of DKFILEs list
	if( !fileptr )
		return -1;						// error FILE struct not found in _dkfile_list (no files open)
		
	while( fileptr->next && fileptr->next!=stream )
		fileptr = fileptr->next;

    if( fileptr == stream )		
		_dkfile_list = stream->next;
	else if( fileptr->next!=stream )
		return -1;						// error - FILE struct not found in _dkfile_list
	else
		fileptr->next = stream->next;		// unlink stream from _dkfile_list
	
	if( stream->filename )
	    free( stream->filename );
	free( stream );
	

	return 0;				
}


//--------------------------------------------------------------------

int dkfseek(DKFILE *stream, off_t offset, int whence)
{
	return stream->io->seek( stream->info, offset, whence );
}


//--------------------------------------------------------------------

long dkftell(DKFILE *stream)
{
	return stream->io->pos( stream->info );
}


//--------------------------------------------------------------------

int dkfgetc(DKFILE *stream)
{
	return stream->io->read_byte( stream->info );
}


//--------------------------------------------------------------------

int dkfputc(int c, DKFILE *stream)
{
	return stream->io->write_byte( stream->info, c );
}


//--------------------------------------------------------------------
//char *fgets(char *restrict s, int n, FILE *restrict stream)
char *dkfgets(char *s, int n, DKFILE *stream)
{
	return NULL;
}


//--------------------------------------------------------------------
//size_t fread(void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream)
size_t dkfread(void *ptr, size_t size, size_t nitems, DKFILE *stream)
{
	int n;
		

	for( n=0; n<nitems && io_ctrl_char!=3; n++ )		// checking io_ctrl_char allows this loop to be broken by ctrl-c
		stream->io->read_bytes( stream->info, size, ptr+(n*size) );

	
	return n;
}       


//--------------------------------------------------------------------
//size_t fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream)
size_t dkfwrite(const void *ptr, size_t size, size_t nitems, DKFILE *stream)
{
	int n;
	

	for( n=0; n<nitems; n++ )
	{
//		for( i=0; i<size; i++ )
//		{
//			dkfputc( ((uint8_t *)ptr)[n*nitems + i], stream );
//		}
		stream->io->write_bytes( stream->info, size, (void *)ptr+(n*size) );
	}

	
	return n;
}


//--------------------------------------------------------------------

int dkfeof(DKFILE *stream)
{
	return stream->io->eof( stream->info );
}


//--------------------------------------------------------------------

int dkferror(DKFILE *stream)
{
	return stream->io->error( stream->info );
}

//--------------------------------------------------------------------
