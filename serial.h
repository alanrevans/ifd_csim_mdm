#ifndef serial_h
#define serial_h

/*
 *
 * $Id: serial.h,v 1.2 1999/06/07 22:37:29 corcoran Exp $
 *
 * NAME:
 *	serial.h - Copyright (C) 1998 David Corcoran
 *             corcordt@cs.purdue.edu
 *
 * DESCRIPTION:
 *      This provides Unix/Mac serial interface drivers.
 *
 * AUTHOR:
 *	David Corcoran, 3/17/98
 *
 * LICENSE: See file LICENSE.
 *
 */

#include "defines.h"

bool
IO_InitializePort(	/* Initialize the card reader port.	*/
	int baud,	/* Baud rate to set port to		*/
	int bits,	/* Bytesize: 5, 6, 7 or 8		*/
	char par,	/* Parity: 'E' even, 'O' odd, 'N' none	*/
	char* port	/* Name of port, or (char *)0L for dialog */
);

HANDLE
IO_ReturnHandle(        /* Returns the current handle           */
	void
);

int 
IO_UpdateReturnBlock(   /* Returns the current blocking time    */
	int blocktime   /* The updated blocking time            */
);

int
IO_ReturnBaudRate(      /* Return the current baudrate          */
	void
);

bool			/* True for sucess, false otherwise	*/
IO_FlushBuffer(		/* Flush buffer to card reader		*/
	void
);

bool
IO_Read(		/* Read up to 256 bytes from the port	*/
	int readsize,   /* Number of bytes to read		*/
	BYTE *response  /* Bytes read                           */
);	

bool    
IO_Write(
	BYTE c          /* Byte to be written                   */
);

bool			/* True for success, false otherwise */
IO_Close(			/* On a Mac, gotta close the port */
	void
);

#endif
