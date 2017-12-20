/*****************************************************************
/
/ File   :   ctapi.c
/ Skeleton :   David Corcoran
/ Changes for 3G/4G Modem Driver : Alan R Evans
/ Date   :   Februari, 1999
/ Purpose:   Defines CT-API functions
/ License:   See file LICENSE
/
******************************************************************/

#include "defines.h"
#include "ctapi.h"
#include "csim_api.h"
#include "csim_raw.h"


/*
 * Defines for this file
 */
#define TRASHBUFLEN 270  /* Length of a buffer big enough for any return */

/*
 *  Static variables for this file
 */
static unsigned char protocol = 0;  /* Protocol of active card */

/*
 * Translates return value from the Intertex reader into ctapi
 * values.
 */
int ixret2ctret(int ixret)
{
  switch (ixret) {
  case IX_ICC_OK:
  case IX_ICC_UNNORMAL_SW1_2: /* Let application detect SW1+SW2 itself */
  case IX_ICC_SW1_2_TOO_EARLY:
    return OK;
  case IX_ICC_ILLEGAL_CMD:
  case IX_ICC_ILLEGAL_PAR:
  case IX_ICC_BAD_LENGTH:
    return ERR_INVALID;
  case IX_ICC_CRD_REMOVED:
    return ERR_CT;
  case IX_ICC_CRD_PAR_ERRORS:
  case ICC_ERR_OPENFAIL:
  case ICC_ERR_DCBFAIL:
  case ICC_ERR_TIMOSETUPFAIL:
  case ICC_ERR_ATSCFAIL:
  case ICC_ERR_MSGFAIL:
  case ICC_ERR_MSGCORRUPT:
  case ICC_ERR_NOTOPEN:
    return ERR_TRANS;
  case ICC_ERR_BUFTOOSMALL:
    return ERR_MEMORY;
  default: /* If nothing else fits */
    return ERR_CT;
  }
}

/* Initializes the port on which the reader resides */

int  CT_init ( unsigned int Ctn,  unsigned int pn ) {
  
  bool BretVal;        /* Return value from IO_InitializePort() */
  int  IretVal;        /* Return for this function */  

/* CHANGE :: You may have to change some of the IO_InitializePort() 
   parameters such as baudrate, parity, and bits.                   */

  switch( pn ) {

  case PORT_COM1:
    BretVal = ICC_Open_Channel("/dev/pcsc/0");
    break;

  case PORT_COM2:
    BretVal = ICC_Open_Channel("/dev/pcsc/1");
    break;

  case PORT_COM3:
    BretVal = ICC_Open_Channel("/dev/pcsc/2");
    break;

  case PORT_COM4:
    BretVal = ICC_Open_Channel("/dev/pcsc/3");
    break;

  default:
   BretVal = ICC_ERR_OPENFAIL;
   break;
  }
  
  if (BretVal) {
    IretVal = ERR_MEMORY;        /* Could not allocate port */
  } else {
    IretVal = OK;
  }
  return IretVal;  
}

/* Closes the port in which the reader resides */

int  CT_close( unsigned int Ctn ) {
  
  if (ICC_Close_Channel() == TRUE) {
    return OK;
  } else {
    return ERR_CT;
  }
}            

/* 
 * Sends/Receives Data to/from the Reader
 * Parameters:
 *      ctn: logical cardterminal number.
 *	dad: destination address.
 *	sad: source address.
 *	lc: cmd length in bytes.
 *	cmd: ICC cmd or CT cmd.
 *	lr: passing of the max. buffer size of the rsp and 
 *	return of the actual length of the rsp.
 * 	rsp: rsp to the cmd.
 * Returns:
 *      OK or CT-API error code.
 */ 

int  CT_data( unsigned int ctn, unsigned char *dad, unsigned char *sad,
              unsigned int  lc, unsigned char *cmd, unsigned int  *lr,
              unsigned char *rsp ) {
  
  /* Reader specific CT-BCS commands */
  

  int IretVal = ERR_INVALID;     /* Return Value. Assume error.    */
  int IXret;                     /* Return from IX API call */
  int lrtmp;                     /* Saves return buffer length temporary */
  unsigned char buf[TRASHBUFLEN]; /* Trash return buffer */
  unsigned char ixcmd;           /* Command echo return */
  unsigned char ixpar;           /* Parameter return */

  if ( *dad == 1 ) {            /* This command goes to the reader */
   IretVal = ERR_INVALID;       /* Invalid SAD/DAD Address */
  } else if ( *dad == 0 ) {      /* This command goes to the card */
    
    // Don't get confused here this is for the return saying
    // the source was the card and the destination the host

    *sad = 0;  /* Source Smartcard */
    *dad = 2;  /* Destination Host */
    
    if (protocol == 0) {
      /*
       * Assume send command if more than <CLA><INS><P1><P2><P3> = 5 bytes
       * otherwise, assume receive.
       *
       * Feels a bit risky (might be some I/O-command cards out there??),
       * but this appears to be how the others are doing it...
       */
      if (lc > 5) 
	IXret = ICC_Data_To_Card(cmd, lc, rsp, lr, 0);
      else
	IXret = ICC_Data_From_Card(cmd, lc, rsp, lr);
    }
    else { /* (T=1 should work, for the others: try it and hope...) */
      IXret = ICC_Data_From_Card(cmd, lc, rsp, lr);
    }
    IretVal= ixret2ctret(IXret);
 } else {
   IretVal = ERR_INVALID;              /* Invalid SAD/DAD Address */
 }
  if (IretVal != OK) {
    *lr = 0;
  }
  return IretVal;
}
