/*****************************************************************
/
/ File   :   csim_raw.c
/ Author :   Mårten Norman
/ Date   :   Februari 1999
/ Purpose:   Talks to +CSIM Smartcard Modem. Provides the "raw"
/            function CSIM_CrdIO().
/ License:   See file LICENSE
/
******************************************************************/

#include <stdio.h>
#include "serial.h"
#include "csim_api.h"
#include "csim_raw.h"

/*
// Variables local to this file
*/

static char            eventflag = 0; /* Signals reader status change */
static unsigned char   checkbyte;     /* Counts frame checksum */

/*
//  Transmits a byte to modem/reader
//
//  Returns:          1 = transmit error
//                    0 = ok
*/
static int putbyte(char c) {
  if (IO_Write(c) == FALSE) return 1;
  return 0;
}

/*
//  Transmits a byte to modem/reader while fixing DLEs + checkbyte
//
//  In:               c  data to send
//
//  Returns:          1 = transmit error
//                    0 = ok
*/
static int ixputbyte(unsigned char c) {
  checkbyte ^= c;
  if (IO_Write(c) == FALSE) return 1;
  if (c == DLE) { if (IO_Write(c) == FALSE) return 1; }
  return 0;
}

/*
//  Receives a byte from modem/reader
//
//  Returns:          -1      = Receive error/timeout
//                    0..0xff = data
*/
static int getbyte() {
  BYTE buffer;
  if (IO_Read(1, &buffer) == FALSE) return -1;
  return buffer;
}

/*
//  Receives a byte from modem/reader while fixing DLEs + checkbyte
//
//  Returns:          -1      = Receive error/timeout
//                    -10     = IX_BEGIN
//                    -20     = IX_END
//                    0..0xff = data
*/
static int ixgetbyte() {
  BYTE buffer;
  if (IO_Read(1, &buffer) == FALSE) return -1;
  if (buffer == DLE) {
    if (IO_Read(1, &buffer) == FALSE) return -1;

    switch (buffer) {
    case STX:
      return IX_BEGIN; /* Don't count start into checkbyte! */
      break;
    case ETX:
      return IX_END;  /* Don't count end into checkbyte! /*
      break;
    case DC4: /* Status change */
    /* Not supported yet. Should not happen unless asked for... */ 
      break;
    case DLE:  /* Just swallow doubled DLE */
    default:
      break;
    }
  }
  checkbyte ^= buffer;
  return buffer;
}

/*
//  Transmits a string to modem/reader
//
//  All <DLE> etc. stuff is performed here.
//  
//  In:               cmd     command byte
//                    par     parameter byte
//                    len     length count of string
//            string  Data to send. Not NULL-terminated (as C strings are)
//  Returns:          1 = transmit error
//                    0 = ok
//
*/
static int String2Crd(
		      int len,
		      const unsigned char *string)
{
  char out[255];
  char *p_out = out;
  int i,t;
  sprintf(out, "AT+CSIM=%d,\"", len*2);
  t = strlen(out);
  for (i = 0; i < len; i++) {
    sprintf(&out[t+(i*2)], "%02X", string[i]);
  }
  sprintf(&out[t+(i*2)], "\"\r\n");
  t = strlen(out);
  while(t-- > 0) {
    if (putbyte(*p_out++)) return 1;
  }
  return 0;
}

/*
//  Transmits a null terminated string to modem/reader
//
//  Note: 
//
//  In:               string  NULL-terminated data to send
//  Returns:          1 = transmit error
//                    0 = ok
//
*/
int String2Mdm(unsigned char *string) {

  while(*string != '\0') {
    if (putbyte(*string++)) return 1;
  }

  return 0;
}

/*
//  Receives a string from modem/reader.
//
//  All <DLE> etc. stuff is performed here.
//  
//
//  In:               cmd     command byte
//                    par     parameter byte
//                    maxlen  Max length allowed for string.
//                    string  Pointer to inbuffer. Will not be used if
//                            maxlen == 0.
//
//  Returns:  Number of bytes received. 
//            -1 if communication error.
//            -2 if maxlen overflow.
//            -3 if checksum error
*/
static int Crd2String(
	       int maxlen,
	       unsigned char *string)
{
  int c, x;
  int len;
  int timeout;

  timeout = 5;     /* Seconds to wait for response */

  char modem_str[255];
  char hex_str[3];
  char *cptr = modem_str;
  char *sptr = string;
  char *len_start, *len_end;
  len = 0;

  while (len < sizeof(modem_str)) { /* Get <DATA>, if any */
    c = getbyte();
    if (c == -1) return -1;
    *cptr++ = c;
    *cptr = '\0';
    len++;
    if (strstr(modem_str, "OK")) {
        printf("<- Modem: %s", modem_str); 
        len_start = strstr(modem_str, "+CSIM: ");
        len_end = strstr(len_start,  ",");
        *len_end = '\0';
        len = atoi(len_start + 7);
        len =  len / 2;
        len_end = len_end + 2;
        hex_str[2] = '\0';
        x = 0;
        while(x < len) {
           hex_str[0] =  *len_end;
           hex_str[1] = *(len_end + 1);
	   sscanf(hex_str, "%X", sptr); 
           len_end = len_end + 2;
           *sptr++;
           x++;
        }
        return len;
    } 
    else if (strstr(modem_str, "ERROR")) return -1;
  }
  return -2;
}

/*
//  Scans input from modem until OK or timeout
//
//
//  Returns:  1 if CONNICC
//            0 if timeout or error
*/
static int GetOK() {

  static unsigned char ok[]="OK";
  unsigned char *ptr = ok;
  int c;

  do {
    c = getbyte();
    if ( c < 0 ) return 0; /* timeout or error */
    if (*ptr == c) ptr++; else ptr = ok;
  } while (*ptr != '\0');

  return 1;
}


static int GetResponse(unsigned char *string, int maxlen) {

  static unsigned char ok[]="OK";
  static unsigned char error[]="ERROR";
  unsigned char *ok_ptr = ok;
  unsigned char *error_ptr = error;
  int c;
  int len = 0;

  c = getbyte();
  while (c > 0 && len < maxlen ) {
    if ( c != '\r'  && c != '\n') {
        *string++ = c;
        len++;
    }
    c = getbyte();
  }
  return len;
}


/*
//  Sends a smart card command to the modem/reader and returns result.
//
//  In:               cmdin   command byte
//                    parin   parameter byte
//                    lin     Length of datain
//                    datain  Pointer to datain buffer
//                    lout    Max length allowed for string
//                    datain  Pointer to datain buffer
//
//  Out:              cmd     Echo of command byte
//                    par     Response code
//                    lout    Length of received data
//
//  Returns:          0 if OK
//                    Otherwise error code.
*/
int CSIM_CrdIO(
	     int lin, 
	     const unsigned char *datain,
	     int *lout,
	     unsigned char *dataout)
{
#ifdef PCSC_DEBUG
  int t;
  printf("--> CSIM reader: ");
  for (t = 0; t < lin; t++)
    printf("%02x ", datain[t]);
  printf("\n");
#endif

  String2Mdm("AT\r\n");
  if (!GetOK()) return ICC_ERR_ATSCFAIL;
  String2Mdm("ATE1\r\n");
  if (!GetOK()) return ICC_ERR_ATSCFAIL;
  if (String2Crd(lin, datain)) return ICC_ERR_MSGFAIL;
  *lout = Crd2String(*lout, dataout);

#ifdef PCSC_DEBUG
  printf("<-- Modem: ");
  for (t = 0; t < *lout; t++)
    printf("%02x ", dataout[t]);
  printf("\n***********************************************************\n");
#endif

  if (*lout == -1) return ICC_ERR_MSGFAIL;
  if (*lout == -2) return ICC_ERR_BUFTOOSMALL;
  if (*lout == -3) return ICC_ERR_MSGCORRUPT;
  return 0; /* OK! */
}

