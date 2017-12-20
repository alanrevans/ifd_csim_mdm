/*****************************************************************
/
/ File   :   ixapi.h
/ Author :   Mårten Norman
/ Date   :   Februari 1999
/ Purpose:   Defines for ix API functions and constants
/ License:   See file LICENSE
/
******************************************************************/

/*
 *  API functions
 */
int ICC_Open_Channel(char *port);
int ICC_Activate_Card(int type, unsigned char *hist, int *histlen);
int ICC_Deactivate_Card();
int ICC_Data_To_Card(unsigned char *datain,
		     int lenin,
		     unsigned char *dataout,
		     int *lenout,
		     int pinoffset
		     );
int ICC_Data_From_Card(unsigned char *datain,
		     int lenin,
		     unsigned char *dataout,
		     int *lenout
		     );
int ICC_Do_Card_Command(unsigned char *datain,
			int lenin,
			unsigned char *dataout,
			int *lenout
			);
int ICC_Close_Channel();

/*
 *  API error codes
 */
#define ICC_ERR_OPENFAIL       1000
#define ICC_ERR_DCBFAIL        1001
#define ICC_ERR_TIMOSETUPFAIL  1002
#define ICC_ERR_ATSCFAIL       1003
#define ICC_ERR_MSGFAIL        1004
#define ICC_ERR_MSGCORRUPT     1005
#define ICC_ERR_BUFTOOSMALL    1006
#define ICC_ERR_NOTOPEN        1007
#define ICC_ERR_NOCMDMODE      1009

#define IX_ICC_OK                 0
#define IX_ICC_DONE_OK          126
#define IX_ICC_CRD_REMOVED      128
#define IX_ICC_CRD_NO_RESPONSE  129
#define IX_ICC_CRD_PAR_ERRORS   130
#define IX_ICC_WRONG_CRD_TYPE   131
#define IX_ICC_ILLEGAL_CMD      133
#define IX_ICC_UNNORMAL_SW1_2   135
#define IX_ICC_ILLEGAL_PAR      136
#define IX_ICC_SW1_2_TOO_EARLY  141
#define IX_ICC_CRD_IS_T0        143
#define IX_ICC_CRD_IS_T1        144
#define IX_ICC_BAD_LENGTH       146
