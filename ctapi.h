/*****************************************************************
/
/ File   :   ctapi.h
/ Author :   David Corcoran
/ Date   :   September 2, 1998
/ Purpose:   Defines CT-API functions and returns
/ License:   See file LICENSE
/
******************************************************************/

#ifndef _ctapi_h_
#define _ctapi_h_

#ifdef __cplusplus
extern "C" {
#endif

int CT_init (
      unsigned int Ctn,                  /* Terminal Number */
      unsigned int pn                    /* Port Number */
      );

int CT_close(
       unsigned int Ctn                  /* Terminal Number */
       );                 

int CT_data( 
       unsigned int ctn,                 /* Terminal Number */
       unsigned char *dad,               /* Destination */
       unsigned char *sad,               /* Source */
       unsigned int  lc,                 /* Length of command */
       unsigned char *cmd,               /* Command/Data Buffer */
       unsigned int  *lr,                /* Length of Response */
       unsigned char *rsp                /* Response */
       );


#define OK               0               /* Success */
#define ERR_INVALID     -1               /* Invalid Data */
#define ERR_CT          -8               /* CT Error */
#define ERR_TRANS       -10              /* Transmission Error */
#define ERR_MEMORY      -11              /* Memory Allocate Error */
#define ERR_HTSI        -128             /* HTSI Error */

#define PORT_COM1	   0             /* COM 1 */
#define PORT_COM2	   1             /* COM 2 */
#define PORT_COM3	   2             /* COM 3 */
#define PORT_COM4	   3             /* COM 4 */
#define PORT_Printer       4             /* Printer Port (MAC) */
#define PORT_Modem         5             /* Modem Port (MAC)   */
#define PORT_LPT1	   6             /* LPT 1 */
#define PORT_LPT2	   7             /* LPT 2 */

#ifdef __cplusplus
}
#endif

#endif
