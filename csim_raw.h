/*****************************************************************
/
/ File   :   ixraw.h
/ Author :   Mårten Norman
/ Date   :   Februari 1999
/ Purpose:   Defines for "raw" Intertex Smartcard Modem interface
/ License:   See file LICENSE
/
******************************************************************/

/* Makes control characters harmless to print */
#define printable(c) (((c) < ' ' | (c) >'~') ? ' ' : (c))
/* Misc. constants for internal use */

#define NUL             0x00   // Null
#define STX             0x02   // Start of text
#define ETX             0x03   // End of text
#define CR              0x0d   // <CR>
#define DLE             0x10   // Smartcard <DLE> character
#define DC4             0x14   // Smartcard event character

#define IX_BEGIN        -10    /* Begin smartcard frame */
#define IX_END          -20    /* End smartcard frame */

/* Exported functions */

int CSIM_CrdIO(
	     int lin, 
	     const unsigned char *datain,
	     int *lout,
	     unsigned char *dataout
	     );
