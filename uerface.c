/*
   uerface.c - linux sample code for voyager
   Copyright ¨Ï 2009. CrasID Co., Ltd. All Rights Reserved. <support@crasid.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
   */
#include "app.h"


int ufFaceDetect(int fd_fifo)
{
	unsigned char* prxdata;
	int rxdatalen;

	unsigned char strbuf[256];
	int pktsize;
	unsigned short cmd;
	int incv;
	int ret;
	int loop0;

	int facecnt = 0;
	int face_left;
	int face_top;
	int face_right;
	int face_bottom;
	int face_cx;
	int face_cy;

	TIME_CHECK_ST();
	commClearBuff();
	pktsize = pktMakeTxData( PKT_CMD_UERFACE_FACECHECK, NULL, 0, 0 );
	if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
		if ( commRecvPktData( 1000 ) > 0 ){
			cmd = pktGetUartRxCmd();
			if ( cmd == (unsigned short)(PKT_CMD_UERFACE_FACECHECK|PKT_REQUEST_OK) ){
				prxdata = pktGetUartRxData();

				incv = 0;
				facecnt = ((prxdata[incv+0]<<24) & 0xff000000) | 
					((prxdata[incv+1]<<16) & 0xff0000) |
					((prxdata[incv+2]<<8) & 0xff00) |
					((prxdata[incv+3]<<0) & 0xff) ;
				incv+= 4;

				logp( "------------ find face : %d fd: %d------------\n", facecnt , fd_fifo);
				for ( loop0=0; loop0< facecnt; loop0++ ){
					face_left = (int)(((prxdata[incv+0]<<8) & 0xff00)|((prxdata[incv+1]<<0) & 0xff));
					face_top = (int)(((prxdata[incv+2]<<8) & 0xff00)|((prxdata[incv+3]<<0) & 0xff));
					face_right = (int)(((prxdata[incv+4]<<8) & 0xff00)|((prxdata[incv+5]<<0) & 0xff));
					face_bottom = (int)(((prxdata[incv+6]<<8) & 0xff00)|((prxdata[incv+7]<<0) & 0xff));
					incv+=8;

					face_cx = face_left + ( (face_right-face_left)>>1 );
					face_cy = face_top + ( (face_bottom-face_top)>>1 );
					logp("[%d] cx : %d, cy : %d, left : %d, top : %d, right : %d, bottom : %d\n", 
							loop0, (face_cx - 120), (face_cy - 160),
							face_left, face_top, face_right, face_bottom); //

					if(fd_fifo != -1){
						logp("Be wrote down in to the FIFO\n");
						dprintf(fd_fifo,"{ \"cnt\": %d, \"cx\" : %d, \"cy\" : %d, \"left\" : %d, \"top\" : %d, \"right\" : %d, \"bottom\" : %d}", 
								loop0, (face_cx - 120), (face_cy - 160),
								face_left, face_top, face_right, face_bottom);
					}else {
						logp("Fifo has some error, %d\n", fd_fifo);
					}
				}
				logp( "---------------------------------------\n" );
			}
		}
	}
	TIME_CHECK_EN();
	return facecnt;
}


int ufFaceRegPre( int timeout )
{
	unsigned char* prxdata;
	int rxdatalen;

	int pktsize;
	unsigned short cmd;
	int incv;
	int ret;
	int loop0;

	unsigned long long st_ms;

	int retcode;
	int yval; // for test
	int face_left;
	int face_top;
	int face_right;
	int face_bottom;
	int face_cx;
	int face_cy;

	int stable=0;

	st_ms = getTimems();

	commClearBuff();
	do{
		TIME_CHECK_ST();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERADD_PREPROCESS, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 1000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERADD_PREPROCESS|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();
					incv = 0;
					retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
						((prxdata[incv+1]<<16) & 0xff0000) |
						((prxdata[incv+2]<<8) & 0xff00) |
						((prxdata[incv+3]<<0) & 0xff) ;
					incv+= 4;

					logp("### retcode : %d ###\n", retcode);
					if ( rxdatalen > 10 ){
						face_left = (int)(((prxdata[incv+0]<<8) & 0xff00)|((prxdata[incv+1]<<0) & 0xff));
						face_top = (int)(((prxdata[incv+2]<<8) & 0xff00)|((prxdata[incv+3]<<0) & 0xff));
						face_right = (int)(((prxdata[incv+4]<<8) & 0xff00)|((prxdata[incv+5]<<0) & 0xff));
						face_bottom = (int)(((prxdata[incv+6]<<8) & 0xff00)|((prxdata[incv+7]<<0) & 0xff));
						incv+=8;

						face_cx = face_left + ( (face_right-face_left)>>1 );
						face_cy = face_top + ( (face_bottom-face_top)>>1 );
						logp("center x : %d\n", (face_cx - 120)); // 
						logp("cetner y : %d\n", (face_cy - 160)); // 
						logp("w: %d, h: %d\n", (face_right-face_left), (face_bottom-face_top)); // 
					}
					yval = ((prxdata[incv+0]<<24) & 0xff000000) | 
						((prxdata[incv+1]<<16) & 0xff0000) |
						((prxdata[incv+2]<<8) & 0xff00) |
						((prxdata[incv+3]<<0) & 0xff) ;
					incv+= 4;
					logp("######### %d #########\n", yval );

					if ( retcode == 0 ){
						++stable;
						if ( stable > 5 ){
							return 0;
						}
					}
					else{
						stable = 0;
					}
				}
			}
		}
		TIME_CHECK_EN();
	}while( (getTimems()-st_ms) < timeout  );
	return -1;
}

int ufFaceReg( int timeout )
{
	unsigned char* prxdata;
	int rxdatalen;

	int pktsize;
	unsigned short cmd;
	int incv;
	int ret;
	int loop0;

	unsigned long long st_ms;

	int retcode;
	int face_left;
	int face_top;
	int face_right;
	int face_bottom;
	int face_cx;
	int face_cy;

	char retId[20];	

	int stable=0;

	st_ms = getTimems();

	do{
		TIME_CHECK_ST();
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERADD, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 1000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERADD|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();
					incv = 0;
					retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
						((prxdata[incv+1]<<16) & 0xff0000) |
						((prxdata[incv+2]<<8) & 0xff00) |
						((prxdata[incv+3]<<0) & 0xff) ;
					incv+= 4;

					logp("### retcode : %d ###\n", retcode);
					if ( rxdatalen > 8 ){
						face_left = (int)(((prxdata[incv+0]<<8) & 0xff00)|((prxdata[incv+1]<<0) & 0xff));
						face_top = (int)(((prxdata[incv+2]<<8) & 0xff00)|((prxdata[incv+3]<<0) & 0xff));
						face_right = (int)(((prxdata[incv+4]<<8) & 0xff00)|((prxdata[incv+5]<<0) & 0xff));
						face_bottom = (int)(((prxdata[incv+6]<<8) & 0xff00)|((prxdata[incv+7]<<0) & 0xff));
						incv+=8;

						face_cx = face_left + ( (face_right-face_left)>>1 );
						face_cy = face_top + ( (face_bottom-face_top)>>1 );
						logp("center x : %d\n", (face_cx - 120)); // 
						logp("cetner y : %d\n", (face_cy - 160)); // 
						logp("w: %d, h: %d\n", (face_right-face_left), (face_bottom-face_top)); // 
					}

					if ( retcode >= 0 ){
						memset( retId, 0, 20 );
						memcpy( retId, &prxdata[incv], rxdatalen-incv ); 
						logp("##### ID : %s #####\n", retId );
						return retcode;
					}
					else if ( retcode == -110 ){  // suspend
						return retcode;
					}
				}
			}
		}
		TIME_CHECK_EN();
	}while( (getTimems()-st_ms) < timeout  );
	return -1;
}

int ufFaceRec( int timeout )
{
	unsigned char* prxdata;
	int rxdatalen;

	int pktsize;
	unsigned short cmd;
	int incv;
	int ret;
	int loop0;

	unsigned long long st_ms;

	int retcode;
	int face_left;
	int face_top;
	int face_right;
	int face_bottom;
	int face_cx;
	int face_cy;

	char retId[20];	

	int stable=0;

	st_ms = getTimems();

	do{
		TIME_CHECK_ST();
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERCHECK, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 1000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERCHECK|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();
					incv = 0;
					retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
						((prxdata[incv+1]<<16) & 0xff0000) |
						((prxdata[incv+2]<<8) & 0xff00) |
						((prxdata[incv+3]<<0) & 0xff) ;
					incv+= 4;

					logp("### retcode : %d ###\n", retcode);
					if ( rxdatalen > 10 ){
						face_left = (int)(((prxdata[incv+0]<<8) & 0xff00)|((prxdata[incv+1]<<0) & 0xff));
						face_top = (int)(((prxdata[incv+2]<<8) & 0xff00)|((prxdata[incv+3]<<0) & 0xff));
						face_right = (int)(((prxdata[incv+4]<<8) & 0xff00)|((prxdata[incv+5]<<0) & 0xff));
						face_bottom = (int)(((prxdata[incv+6]<<8) & 0xff00)|((prxdata[incv+7]<<0) & 0xff));
						incv+=8;

						face_cx = face_left + ( (face_right-face_left)>>1 );
						face_cy = face_top + ( (face_bottom-face_top)>>1 );
						logp("center x : %d\n", (face_cx - 120)); // 
						logp("cetner y : %d\n", (face_cy - 160)); // 
						logp("w: %d, h: %d\n", (face_right-face_left), (face_bottom-face_top)); // 
					}

					if ( retcode >= 0 ){
						memset( retId, 0, 20 );
						memcpy( retId, &prxdata[incv], rxdatalen-incv ); 
						logp("##### ID : %s #####\n", retId );
						return retcode;
					}
				}
			}
		}
		TIME_CHECK_EN();
	}while( (getTimems()-st_ms) < timeout  );
	return -1;
}

int ufFaceRecFr( int timeout )
{
	unsigned char* prxdata;
	int rxdatalen;

	int pktsize;
	unsigned short cmd;
	int incv;
	int ret;
	int loop0;

	unsigned long long st_ms;

	int retcode;
	int face_left;
	int face_top;
	int face_right;
	int face_bottom;
	int face_cx;
	int face_cy;

	char retId[20];	

	char txpkt[256];

	int stable=0;



	// set freerun timeout
	txpkt[0] = (timeout>>24) & 0xff;
	txpkt[1] = (timeout>>16) & 0xff;
	txpkt[2] = (timeout>>8) & 0xff;
	txpkt[3] = (timeout>>0) & 0xff;
	pktsize = pktMakeTxData( PKT_CMD_UERFACE_FREERUN_TIMEOUT, txpkt, 4, 0 );
	if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
		if ( commRecvPktData( 1000 ) > 0 ){
			cmd = pktGetUartRxCmd();
			if ( cmd == (unsigned short)(PKT_CMD_UERFACE_FREERUN_TIMEOUT|PKT_REQUEST_OK) ){
			}
		}
	}

	// start freerun
	commClearBuff();
	pktsize = pktMakeTxData( PKT_CMD_UERFACE_FREERUN_USERCHECK, NULL, 0, 0 );
	if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
	}

	st_ms = getTimems();

	do{
		TIME_CHECK_ST();
		commClearBuff();

		if ( commRecvPktData( 1000 ) > 0 ){
			cmd = pktGetUartRxCmd();
			if ( cmd == (unsigned short)(PKT_CMD_UERFACE_FREERUN_USERCHECK|PKT_REQUEST_OK) ){
				prxdata = pktGetUartRxData();
				rxdatalen = pktGetUartRxDatalen();
				incv = 0;
				retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
					((prxdata[incv+1]<<16) & 0xff0000) |
					((prxdata[incv+2]<<8) & 0xff00) |
					((prxdata[incv+3]<<0) & 0xff) ;
				incv+= 4;

				if (retcode == -50) { // ERR_TIMEOUT
					logp("-- retcode : %d -- TIMEOUT.\r\n", retcode);
					return -50;
				}

				logp("### retcode : %d ###\n", retcode);
				if ( rxdatalen > 10 ){
					face_left = (int)(((prxdata[incv+0]<<8) & 0xff00)|((prxdata[incv+1]<<0) & 0xff));
					face_top = (int)(((prxdata[incv+2]<<8) & 0xff00)|((prxdata[incv+3]<<0) & 0xff));
					face_right = (int)(((prxdata[incv+4]<<8) & 0xff00)|((prxdata[incv+5]<<0) & 0xff));
					face_bottom = (int)(((prxdata[incv+6]<<8) & 0xff00)|((prxdata[incv+7]<<0) & 0xff));
					incv+=8;

					face_cx = face_left + ( (face_right-face_left)>>1 );
					face_cy = face_top + ( (face_bottom-face_top)>>1 );
					logp("center x : %d\n", (face_cx - 120)); // 
					logp("cetner y : %d\n", (face_cy - 160)); // 
					logp("w: %d, h: %d\n", (face_right-face_left), (face_bottom-face_top)); // 
				}

				if ( retcode >= 0 ){
					memset( retId, 0, 20 );
					memcpy( retId, &prxdata[incv], rxdatalen-incv ); 
					logp("##### ID : %s #####\n", retId );
					return retcode;
				}
			}
		}
		TIME_CHECK_EN();
	}while( (getTimems()-st_ms) < (timeout+1000)  );  // prevent loss resp-pkt

	return -1;
}

int ufLoadUserList()
{
	unsigned char* prxdata;
	int rxdatalen;

	int i;

	unsigned char strbuf[512];
	unsigned char txbuff[4];
	int pktsize;
	unsigned short cmd;
	int incv;
	int ret;
	int loop0;
	int loop1;

	int usercnt = 0;

	// check usercount;
	commClearBuff();
	pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERLIST, NULL, 0, 0 );
	if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
		if ( commRecvPktData( 1000 ) > 0 ){
			cmd = pktGetUartRxCmd();
			if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERLIST|PKT_REQUEST_OK) ){
				prxdata = pktGetUartRxData();
				rxdatalen = pktGetUartRxDatalen();

				incv = 0;
				usercnt = ((prxdata[incv+0]<<24) & 0xff000000) | 
					((prxdata[incv+1]<<16) & 0xff0000) |
					((prxdata[incv+2]<<8) & 0xff00) |
					((prxdata[incv+3]<<0) & 0xff) ;				
			}
		}
	}

	logp( "------------ find user : %d ------------\n", usercnt );

	if ( usercnt <= 0 ){
		return 0;
	}

	// get userlist of page  ( max 10 users in page)
	for( i=0; i< ((usercnt-1)/10)+1; i++ ){
		txbuff[0]=(i>>24)&0xff;
		txbuff[1]=(i>>16)&0xff;
		txbuff[2]=(i>>8)&0xff;
		txbuff[3]=(i>>0)&0xff;

		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERLIST, txbuff, 4, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 1000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERLIST|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();

					// usercnt ignore 
					incv = 4;
					memset( strbuf, 0, 512 );
					memcpy( strbuf, &prxdata[incv], rxdatalen-4 );

					//logp( "- %s -\n", strbuf );
					for( loop0=0,loop1=0; loop0 < 10; loop0++ ){
						if ( strbuf[loop1] == '\0' )
							break;

						logp(" %d. ", ((loop0+1)+(i*10)));
						do{
							if ( strbuf[loop1] == '\0' ){
								break;
							}
							else if ( strbuf[loop1] == (unsigned char)0x2b ){
								logp("\n");
								++loop1;
								break;
							}
							else{
								logp("%c",strbuf[loop1] );
								++loop1;
							}
						}while(1);
					}

				}
			}
		}		
	}
	logp( "---------------------------------------\n" );

	return usercnt;
}

