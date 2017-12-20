/*
 * NAME:
 * 	serial.c -- Copyright (C) 1998 David Corcoran
 *
 * DESCRIPTION:
 *      This provides Unix/Mac serial driver support
 * 
 * AUTHOR:
 *	David Corcoran, 7/22/98
 *
 *	Modified by Mark Hartman for Macintosh support, 7/15/98
 *
 * LICENSE: See file LICENSE.
 *
 */

#include <stdio.h>                       /* Standard Includes     */
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "serial.h"

#ifdef CPU_MAC_PPC                        /* For Macintosh Support */

 #include <CTBUtilities.h>
 #include <CRMSerialDevices.h>
 #include <ConnectionTools.h>
 #include <CommResources.h>

#endif

#ifdef CPU_ICAP_PC                       /* For Unix Support      */

#include <termios.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/time.h>

#endif

struct IO_Specs {
	HANDLE handle;
	BYTE   baud;
	BYTE   bits;
	char   parity;
	long   blocktime;
} ioport;

#ifdef CPU_MAC_PPC

 typedef struct SerialPortData
  {
    ConnHandle		pConn;		    /* CommToolbox connection handle */
  } SerialPortData, *SPPtr, **SPHdl;

#endif

static char _rcsid[] UNUSED = "$Id: serial.c,v 1.2 1999/06/07 22:37:29 corcoran Exp $";

/*
 * InititalizePort -- initialize a serial port
 *	This functions opens the serial port specified and sets the
 *	line parameters baudrate, bits per byte and parity according to the
 *	parameters.  If no serial port is specified, the Communications
 *	Manager's dialog is invoked to permit the user to select not only
 *	the port, but all the other (baud/bits/parity) details as well.
 *	Selections made by the user in this case will override the
 *	parameters passed.
 */

bool
IO_InitializePort(int baud, int bits, char parity, char* port)
{

#ifdef CPU_MAC_PPC

  HANDLE handle;
  SPPtr sp;
  CMErr theErr;
  short procID;
  CMBufferSizes buf;
  register short i;
  Point pt = {100,100};
  char config[512], cItem[64];
  
  InitCRM();				          /* Init the Comm Resource Manager             */
  theErr = InitCM();			          /* And the Communications Manager             */
  if (theErr != cmNoErr)			  /* If we got an error, no tools available     */
    return FALSE;			          /* So send back the default error             */
  
  procID = CMGetProcID("\pSerial Tool");	  /* We want to use the Serial Tool             */
  if (procID == -1)			          /* If it's not there...                       */
    return FALSE;			          /* Send back the default error                */
  
  for (i=0; i <= 7; buf[i++] = 0)		  /* Clear the CTB bufsizes array               */
    ;
  
  handle = NewHandle(sizeof(SerialPortData));     /* Allocate our memory for the port           */
  HLock(handle);				  /* And lock it down                           */
  sp = (SPPtr)*handle;				  /* Dereference it                             */
  
  sp->pConn = CMNew(procID, (cmNoMenus | cmQuiet), buf, 0L, 0L);  /* Create a connection          */
  if (!sp->pConn)						  /* If it could not be created...*/
    {
      DisposeHandle(handle);					  /* Release the memory and...    */
      return FALSE;						  /* Send back the default error  */
    }
  
  if (!port[0])					   /* If no port name was passed                */
    {
      theErr = CMChoose(&sp->pConn, pt, 0L);	   /* If the user didn't specify, dialog        */
      switch (theErr)				   /* What did we get back?                     */
	{
	case chooseDisaster:		   /* Total screwup                 */
	  DisposeHandle(handle);	   /* Release the memory and...     */
	  return FALSE; 		   /* Send back the default error   */
	  break;
	  
	case chooseOKMinor:		   /* Minor changes */
	case chooseOKMajor:		   /* Major changes */
	  theErr = cmNoErr;	           /* All OK        */
	  break;
	  
	default:			   /* Other error conditions */
	  theErr = -1;		           /* Generic "no way" error */
	  break;
	}
    }
  else
    {
      config[0] = 0;			       /* Null out the string      */
      sprintf(cItem, "Baud %d ", baud);	       /* Get the baud rate        */
      strcat(config, cItem);		       /* Tack it on               */
      sprintf(cItem, "dataBits %d ", bits);    /* And the bits per char    */
      strcat(config, cItem);		       /* Tack it on               */
      strcat(config, "Parity ");	       /* Start the parity stuff   */
      switch (parity)
	{
	case 'O':
	case 'o':
	  strcat(config, "Odd ");
	  break;
	  
	case 'E':
	case 'e':
	  strcat(config, "Even ");
	  break;
	  
	default:
	  strcat(config, "None ");
	  break;
	}
      sprintf(cItem, "Port \"%s\"", port);	                /* Get the name of the port    */
      strcat(config, cItem);					/* Tack it on                  */
      theErr = CMSetConfig(sp->pConn, config);                  /* Set the configuration       */
    }
  if (theErr != cmNoErr)					/* If we got an error          */
    {
      CMDispose(sp->pConn);					/* Dispose the connection...   */
      DisposeHandle(handle);					/* Release the memory and...   */
      return FALSE;						/* Send back the default error */
    }
  
  theErr = CMOpen(sp->pConn, FALSE, 0L, -1);	         /* Open the connection (synchronous)  */
  if (theErr != cmNoErr)				 /* If we got an error                 */
    {
      CMDispose(sp->pConn);				 /* Dispose the connection...          */
      DisposeHandle(handle);				 /* Release the memory and...          */
      return FALSE;					 /* Send back the default error        */
    }
  
  ioport.handle = handle;				 /* handle for iostructure             */
  HUnlock(handle);					 /* Unlock the handle                  */
  return TRUE;  					 /* And do so                          */

#endif

                                 /* UNIX SERIAL SUPPORT */

#ifdef CPU_ICAP_PC

  HANDLE handle; 
  struct termios newtio;
  
  handle = open(port, O_RDWR | O_NOCTTY);               /* Try user input depending on port */

    if (handle < 0) {                                   /* Problems with /dev/smartcard     */
      return FALSE;
    }

  /*
   * Zero the struct to make sure that we don't use any garbage
   * from the stack.
   */

  memset(&newtio, 0, sizeof(newtio));

  /*
   * Set the baudrate
   */

  switch (baud) {

  case 9600:                                               /* Baudrate 9600          */
    newtio.c_cflag = B9600;
    break;
  case 19200:                                              /* Baudrate 19200         */
    newtio.c_cflag = B19200;
    break;
  case 38400:                                           /* Baudrate 38400  */
    newtio.c_cflag = B38400;
    break;
  case 57600:                                           /* Baudrate 57600  */
    newtio.c_cflag = B57600;
    break;
  case 115200:                                          /* Baudrate 115200 */
    newtio.c_cflag = B115200;
    break;
  case 230400:                                          /* Baudrate 230400 */
    newtio.c_cflag = B230400;
    break;

  default:
    close(handle);
    return FALSE;

  }
  
  /*
   * Set the bits.
   */
	switch (bits) {
	case 5:                                          /* Five bits             */
	  newtio.c_cflag |= CS5;
	  break;
	case 6:                                          /* Six bits              */
	  newtio.c_cflag |= CS6;
	  break;
	case 7:                                          /* Seven bits            */
		newtio.c_cflag |= CS7;
		break;
	case 8:                                          /* Eight bits            */
	  newtio.c_cflag |= CS8;
	  break;
	default:
	  close(handle);
	  return FALSE;
	}

	/*
	 * Set the parity (Odd Even None)
	 */

	switch (parity) {

	case 'O':                                              /* Odd Parity               */
	  newtio.c_cflag |= PARODD | PARENB | INPCK;
	  break;
	  
	case 'E':                                              /* Even Parity             */ 
	  newtio.c_cflag &= (~PARODD); 
	  newtio.c_cflag |= PARENB | INPCK;
	  break;
		
	case 'N':                                              /* No Parity               */
	  break;
	  
	default:
	  close(handle);
	  return FALSE;
	}
	
	/*
	 * Setting Raw Input and Defaults
	 */

	newtio.c_cflag |= CREAD|HUPCL|CLOCAL;
	newtio.c_iflag &= ~(IGNPAR|PARMRK|INLCR|IGNCR|ICRNL);
	newtio.c_iflag |= BRKINT;  
	newtio.c_lflag &= ~(ICANON|ECHO|ISTRIP);
	newtio.c_oflag  = 0;  
	newtio.c_lflag  = 0;
	
	newtio.c_cc[VMIN]  = 1;
	newtio.c_cc[VTIME] = 0;
	
	if (tcflush(handle, TCIFLUSH) < 0) {             /* Flush the serial port            */
	  close(handle);
	  return FALSE;
	}
	
	if (tcsetattr(handle, TCSANOW, &newtio) < 0) {   /* Set the parameters                */
	  close(handle);
	  return FALSE;
	}
	
	ioport.handle = handle;                           /* Record the handle                 */
	ioport.baud   = baud;                             /* Record the baudrate               */
	ioport.bits   = bits;                             /* Record the bits                   */
	ioport.parity = parity;                           /* Record the parity                 */
	ioport.blocktime = 10;                            /* Default Beginning Blocktime       */
	
	return TRUE;

#endif

}

HANDLE
IO_ReturnHandle() {
  return ioport.handle;                                   /* Return the current used handle    */
}

int 
IO_UpdateReturnBlock(int blocktime) {                    /* Sets the blocking timeout value    */

 #ifdef CPU_MAC_PPC
  ioport.blocktime = (long)blocktime * 60;               /* For Mac multiply 60 for seconds     */
 #endif
  
 #ifdef CPU_ICAP_PC
  ioport.blocktime = (long)blocktime;                    /* For Unix seconds by default         */
 #endif
  
  return ioport.blocktime;                               /* Return the current blocktime        */
}

int
IO_ReturnBaudRate() {
  return ioport.baud;                                    /* Return the current baudrate         */
}

bool
IO_FlushBuffer() {
  
#ifdef CPU_MAC_PPC

  SPPtr sp;
  CMErr theErr = cmNoErr;
  char theBuf[32];
  long charsToRead;
  CMFlags rFlags;
  HANDLE handle;

  handle = ioport.handle;                                         /* Get the handle                */

  HLock(handle);						  /* Lock down the handle          */
  sp = (SPPtr)*handle;						  /* And dereference it            */
  
  while (theErr == cmNoErr)
    {
      charsToRead = 31;				                  /* Read up to 31 characters      */
      theErr = CMRead(sp->pConn, theBuf,
		      &charsToRead, cmData, FALSE,
		      0L, 0L, &rFlags);	                          /* Attempt a one-shot read       */
    }
  HUnlock(handle);
  return TRUE;

#endif

#ifdef CPU_ICAP_PC

  HANDLE handle = ioport.handle;

  if (tcflush(handle, TCIFLUSH) == 0)
    return TRUE;
  return FALSE;

#endif

}


bool
IO_Read( int readsize, BYTE *response ) {
  
#ifdef CPU_MAC_PPC

  SPPtr sp;
  CMErr theErr = cmNoErr;
  long count = readsize;
  CMFlags rFlags;
  HANDLE handle;
  long blocktime;
  bool BretVal;

  handle = ioport.handle;                                       /* Get the handle                          */
  blocktime = ioport.blocktime;                                 /* Get the blocktime                       */
  
  HLock(handle);						/* Lock down the handle                    */
  sp = (SPPtr)*handle;						/* And dereference it                      */
  
  theErr = CMRead(sp->pConn,response,
		  &count, cmData, FALSE,
		  0L, blocktime, &rFlags);		        /* Attempt a one-shot read                 */
  
  BretVal = (theErr == cmNoErr && count != 0);                  /* No error and data present = success     */
  HUnlock(handle);						/* Unlock the handle                       */
  
  return BretVal;						/* And return the success                  */

#endif

#ifdef CPU_ICAP_PC

  fd_set rfds;
  struct timeval tv;
  int rval;
  BYTE c;
  HANDLE handle;
  int count = 0;
  
  handle = ioport.handle;

  tv.tv_sec = ioport.blocktime;
  tv.tv_usec = 0;
  
  FD_ZERO(&rfds);
  
  for (count=0; count < readsize; count++) {
    FD_SET(handle, &rfds);
    rval = select (handle+1, &rfds, NULL, NULL, &tv);
    
    if (FD_ISSET(handle, &rfds)) {
      read(handle, &c, 1);
      response[count] = c;
    }
    else {
      tcflush(handle, TCIFLUSH);
      return FALSE;
    }
  }
  return TRUE;
  
#endif
 
}

bool
IO_Write(BYTE c) {

#ifdef CPU_MAC_PPC

  SPPtr sp;
  HANDLE handle;
  CMErr theErr = cmNoErr;
  long count = 1;
  
  handle = ioport.handle;
  
  IO_FlushBuffer(handle);					/* Flush before write                     */
  HLock(handle);						/* Lock down the handle                   */
  sp = (SPPtr)*handle;						/* And dereference it                     */
  
  theErr = CMWrite(sp->pConn, &c, &count,
		   cmData, FALSE, 0L, 0L, 0L);	                /* Write a single character to the port   */
  HUnlock(handle);						/* Unlock the handle                      */
  return (theErr == cmNoErr);					/* And return the boolean success flag    */

#endif

#ifdef CPU_ICAP_PC

  HANDLE handle = ioport.handle;

  tcflush(handle, TCIFLUSH);                       /* Flush the port      */
  if (write(handle, &c, 1) == 1)                   /* Write one byte      */
    return TRUE;
  return FALSE;
  
#endif

}


bool
IO_Close() {

#ifdef CPU_MAC_PPC

  SPPtr sp;
  HANDLE handle;
  CMErr theErr = cmNoErr;

  handle = ioport.handle;

  HLock(handle);						/* Lock down the handle          */
  sp = (SPPtr)*handle;					        /* And dereference it            */
  
  theErr = CMClose(sp->pConn, FALSE,
		   0L, -1L, TRUE);	                        /* Close the connection now (kill I/O) */
  CMDispose(sp->pConn);						/* Dispose of the connection record    */
  HUnlock(handle);						/* Purely for form                     */
  DisposeHandle(handle);					/* Dispose of the handle               */
  return TRUE;							/* And we're done                      */

#endif

#ifdef CPU_ICAP_PC

  HANDLE handle = ioport.handle;

  if ( close (handle) == 0 ) {                      /* Close the handle */
    return TRUE;
  } else {
    return FALSE;                                  /* Done !            */
  }

#endif

}
