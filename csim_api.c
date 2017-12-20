/*****************************************************************
/
/ File   :   ixapi.c
/ Author :   Mårten Norman
/ Date   :   Februari 1999
/ Purpose:   Provides the +CSIM API
/ License:   See file LICENSE
/
******************************************************************/

#include <stdio.h>
#include "serial.h"
#include "csim_api.h"
#include "csim_raw.h"


/*
//  Connects to modem/reader
//
//  In:               port    String with filename of port
//                            (example: "/dev/ttyS1")
//                            NOTE: The windows API uses
//                                  an int here, for COM
//                                  port number instead.
//
//  Returns:          0 if OK
//                    Otherwise error code.
*/
int ICC_Open_Channel(char *port)
{
  int speed;
  /*
   *  57600 is fastest recommended speed in command mode for ix36.
   *  The modem handles 115200 and 230400, but real life as the cable
   *  and UART RS232 driver timing can give random autobaud misses on
   *  some machines.  And with one autobaud/command and long timeouts
   *  the risk isn't worth it.
   *
   *  This will be no problem with the COM-port sharing driver...
   *
   *  The IX2 PCMCIA reader and IX56 PCMCIA modem/reader has no RS232
   *  cable, and 115200 or 230400 is recommended.
   *  230k needs a Rockwell compatible UART driver though, since an
   *  ordinary 16550 has 115k as max speed.
   */
#ifdef IX56
  speed = 115200;
#else
  speed = 57600;
#endif

  if (IO_InitializePort(speed,8,'E',port) != TRUE) return ICC_ERR_OPENFAIL;
  IO_UpdateReturnBlock(1); /* 1 second timeout is long enough! */

#ifdef PCSC_DEBUG
  printf("3G/4G modem (AT+CSIM) driver version 0.0.1\n");
  printf("Opened IO port baud rate %d\n", speed);
#endif

  return 0;
}

/*
//  Activates card
//
//  In:               type    Protocol according to ISO 7816-3
//                    hist    Points to place to store historical bytes
//                    histlen Max allowed number of historical bytes
//                            
//                            
//
//  Returns:          0 if OK
//                    Otherwise error code.
*/
int ICC_Activate_Card(int type, unsigned char *hist, int *histlen)
{
  return CSIM_CrdIO(0, "", histlen, hist);
}

/*
//  Deactivates card
//
//
//  Returns:          0 if OK
//                    Otherwise error code.
*/
int ICC_Deactivate_Card()
{
  unsigned char buf;
  int buflen = 1;
  
  return CSIM_CrdIO(0, "", &buflen, &buf);
}

/*
//  Transmits data to card
//
//  In:               datain    Data to send
//                    lenin     Length of data
//                    dataout   Buffer for received data
//                    lenout    Pointer to buffer size        
//                    pinoffset Where in buffer to insert PIN       
//
//  Returns:          0 if OK
//                    Otherwise error code.
//                    lenout  Number of received bytes
//                    
*/
int ICC_Data_To_Card(unsigned char *datain,
		     int lenin,
		     unsigned char *dataout,
		     int *lenout,
		     int pinoffset
		     )
{
  return CSIM_CrdIO(lenin, datain, lenout, dataout);
}

/*
//  Receives data from card
//
//  In:               datain    Data to send
//                    lenin     Length of data
//                    dataout   Buffer for received data
//                    lenout    Pointer to buffer size        
//
//  Returns:          0 if OK
//                    Otherwise error code.
//                    lenout  Number of received bytes
//                    
*/
int ICC_Data_From_Card(unsigned char *datain,
		     int lenin,
		     unsigned char *dataout,
		     int *lenout
		     )
{
  return CSIM_CrdIO(lenin, datain, lenout, dataout);
}

/*
//  Makes a "raw" card or reader command
//
//  In:               datain    Data to send
//                    lenin     Length of data
//                    dataout   Buffer for received data
//                    lenout    Pointer to buffer size        
//
//  Returns:          0 if OK
//                    Otherwise error code.
//                    lenout  Number of received bytes
//                    
*/
int ICC_Do_Card_Command(unsigned char *datain,
			int lenin,
			unsigned char *dataout,
			int *lenout
			)
{
  int t;
  
  if ((lenin < 2) || (*lenout < 2)) return 146; /* Bad length error */

  *lenout -= 2;
  t = CSIM_CrdIO(lenin, datain,
	       lenout, dataout);
  return t;
}


/*
//  Closes connection to modem/reader
//
//  Returns:          0
*/
int ICC_Close_Channel()
{
  return IO_Close();
}

