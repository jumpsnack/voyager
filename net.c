/*
  net.c - linux sample code for voyager
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
#include <stdio.h>
#include "app.h"

typedef struct _pkt_net_info{
	int size;
	unsigned int type;
	unsigned int frag;
	int crc;
}PktNetInfo;

static int netSendDataRaw( PktNetInfo* info, unsigned char* data );

int netRecvUData(char* data, int type, int frag, int size, int crc)
{
	PktNetInfo pktinfo;
	int pktsize;
	
	pktinfo.size = size;
	pktinfo.type = type;
	pktinfo.crc = crc;
	pktinfo.frag = frag;
	
	pktsize = pktMakeTxData( PKT_CMD_TRANSFER_READY, NULL, 0, 0 );
	if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){			
		if ( netRecvDataRaw( &pktinfo, data ) > 0 ){
			return pktinfo.size;
		}
	}
	
  return -1;
}


int netSendData( unsigned char* data, int size, unsigned int recv0, unsigned int recv1 )
{
	PktNetInfo pktinfo;
	char respstr[128];
	int incv=0;
	int crc=0;
	int pktsize;

	unsigned char* prxdata;
	unsigned short cmd;
	
	if ( data == NULL ){

	}
	else{
		logp(" data size : %d\n", size );
		pktinfo.size = size;
		respstr[incv++] = (pktinfo.size>>24) & 0xff;
		respstr[incv++] = (pktinfo.size>>16) & 0xff;
		respstr[incv++] = (pktinfo.size>>8) & 0xff;
		respstr[incv++] = (pktinfo.size>>0) & 0xff;

		pktinfo.type = 0;
		respstr[incv++] = (pktinfo.type>>24) & 0xff;
		respstr[incv++] = (pktinfo.type>>16) & 0xff;
		respstr[incv++] = (pktinfo.type>>8) & 0xff;
		respstr[incv++] = (pktinfo.type>>0) & 0xff;

		pktinfo.frag = 0;
		respstr[incv++] = (pktinfo.frag>>24) & 0xff;
		respstr[incv++] = (pktinfo.frag>>16) & 0xff;
		respstr[incv++] = (pktinfo.frag>>8) & 0xff;
		respstr[incv++] = (pktinfo.frag>>0) & 0xff;

		crc = pktGetCrc32( data, size );
		pktinfo.crc = crc;
		respstr[incv++] = (pktinfo.crc>>24) & 0xff;
		respstr[incv++] = (pktinfo.crc>>16) & 0xff;
		respstr[incv++] = (pktinfo.crc>>8) & 0xff;
		respstr[incv++] = (pktinfo.crc>>0) & 0xff;

		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_SEND_FILE_INFO, respstr, incv, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				if ( (unsigned short)pktGetUartRxCmd() == (unsigned short)(PKT_CMD_SEND_FILE_INFO|PKT_REQUEST_OK) ){
					
					commClearBuff();
					if ( commRecvPktData( 1000 ) > 0 ){
						cmd = pktGetUartRxCmd();
						if ( cmd == (unsigned short)(PKT_CMD_TRANSFER_READY) ){
							logd("send data raw\n");
							if ( netSendDataRaw( &pktinfo, data ) > 0 ){
								return size;
							}
						}
					}
				}
			}
		}
	}
	return -1;
}


static int netSendDataRaw( PktNetInfo* info, unsigned char* data )
{
	int ret;
	int incv=0;
	int i, k;
	int ntry;
	
	unsigned short cmd;
	unsigned char txbuff[256];
	
	int pktsize;

	int frameofsec;
	int framecrc;
	int bytespersec;
	int sendsize;
	int sendextsize;

	bytespersec =  (commGetFrameLength());
	frameofsec = ((info->size -1) / bytespersec);
	sendsize = 0;
	memset ( txbuff, 0, 256 );

	for( i=0; i<frameofsec; i++ ){
		commClearBuff();
		// send frame size
		commWriteByte( (char)((bytespersec >> 24) & 0xff) );
		commWriteByte( (char)((bytespersec >> 16) & 0xff) );
		commWriteByte( (char)((bytespersec >> 8) & 0xff) );
		commWriteByte( (char)((bytespersec >> 0) & 0xff) );

		framecrc = 0;
		for( k=0; k<bytespersec; k++ ){
			framecrc += (int)data[sendsize];
			commWriteByte( data[sendsize++] );
		}

		// send frame crc
		commWriteByte( (char)((framecrc >> 24) &0xff));
		commWriteByte( (char)((framecrc >> 16) &0xff));
		commWriteByte( (char)((framecrc >> 8) &0xff));
		commWriteByte( (char)((framecrc >> 0) &0xff));
		if ( commRecvPktData( 1000 ) > 0 ){
			cmd = pktGetUartRxCmd();
			if ( (unsigned short)PKT_CMD_RESEND == cmd  ){
				logd( "request resend\n");
				sendsize -= bytespersec;
				--i;
			}
			else if ( (unsigned short)(PKT_CMD_TRANSFER_DATA|PKT_REQUEST_OK) != cmd ){
				logd( "unknown cmd data\n");
				return -1;
			}
		}
		else{
			logd( "send error\n");
			return -1;
		}
	}

	if ( sendsize < info->size ){
		commClearBuff();
		// send frame size
		sendextsize = (info->size-sendsize);
		i=1;
		do{
			commWriteByte( (sendextsize >> 24) &0xff);
			commWriteByte( (sendextsize >> 16) &0xff);
			commWriteByte( (sendextsize >> 8) &0xff);
			commWriteByte( (sendextsize >> 0) &0xff);

			framecrc = 0;
			for( k=0; k<sendextsize; k++ ){
				framecrc += (int)data[sendsize];
				commWriteByte( data[sendsize++] );
			}

			// send frame crc
			commWriteByte( (framecrc >> 24) &0xff);
			commWriteByte( (framecrc >> 16) &0xff);
			commWriteByte( (framecrc >> 8) &0xff);
			commWriteByte( (framecrc >> 0) &0xff);

			if ( commRecvPktData( 1000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( (unsigned short)PKT_CMD_RESEND == cmd ){
					logd( "request resend\n");
					sendsize -= bytespersec;
					++i;
				}
				else if ( (unsigned short)(PKT_CMD_TRANSFER_DATA|PKT_REQUEST_OK) != cmd ){
					logd( "known cmd data\n");
					return -1;
				}
			}
			else{
				logd( "send error\n");
				return -1;
			}
		}while(--i);
	}

	if ( commRecvPktData( 10000 ) > 0 ){
		cmd = pktGetUartRxCmd();
		if ( cmd == (unsigned short)(PKT_CMD_TRANSFER_COMPLETE|PKT_REQUEST_OK) ){
		}
		else{
		}
	}
			
	return info->size;
}

/****************************************
*
*/
int netRecvDataRaw( PktNetInfo* info, unsigned char* data )
{
	unsigned char* prxdata;
	int rxdatalen;

	int pktsize;
	unsigned short cmd;
	int incv;
	int bytesPerSize;

	int rxdataframecrc;
	int framecrc;
	int rxdataframesize;
	char* rxbuff;
	int rxpktTlen;
	int rxpktlen;
	unsigned long long st_ms;

	int  dataTlen;

	// receive data
	st_ms = getTimems();
	rxbuff = malloc (UART_MAX_RX_BUFF);
	rxpktTlen = 0;
	dataTlen = 0;

	TIME_CHECK_ST();

	commClearBuff();
	do{
		while ( (rxpktlen=commRead(&rxbuff[rxpktTlen])) <= 0 ){
			if ( (getTimems()-st_ms) >= TIMEOUT_STREAMDATA ) {
				logd("err-time over [10sec].\n");
				free(rxbuff);
				return -1;
			}
		};
		
		rxpktTlen += rxpktlen;
		if ( rxpktTlen < 4 ){
			continue;
		}

		printf(" 0x%x 0x%x 0x%x 0x%x \n", rxbuff[0],rxbuff[1],rxbuff[2],rxbuff[3]);
		rxdataframesize = ((rxbuff[0]<<24) & 0xff000000) | 
				((rxbuff[1]<<16) & 0xff0000) |
				((rxbuff[2]<<8) & 0xff00) |
				((rxbuff[3]<<0) & 0xff) ;
		logd("0rx datasize: %d - framesize : %d \n", rxpktTlen, rxdataframesize);
		//load full data
		while ( rxpktTlen < rxdataframesize+8 ){
			if ( (rxpktlen=commRead(&rxbuff[rxpktTlen])) > 0 ){
				rxpktTlen += rxpktlen;
			}
			else if ( (getTimems()-st_ms) >= TIMEOUT_STREAMDATA ) {
				logd("err-time over [10sec].\n");
				free(rxbuff);
				return -1;
			}
		};

		rxdataframecrc = ((rxbuff[rxdataframesize+4]<<24) & 0xff000000) | 
					((rxbuff[rxdataframesize+5]<<16) & 0xff0000) |
					((rxbuff[rxdataframesize+6]<<8) & 0xff00) |
					((rxbuff[rxdataframesize+7]<<0) & 0xff) ;
		framecrc = pktGetCrc32( &rxbuff[4], rxdataframesize);
		logd("1rx datasize : %d - framesize : %d - crc[0x%x] : 0x%x\n", rxpktTlen, rxdataframesize, framecrc, rxdataframecrc);
		commClearBuff();

		
		if ( (getTimems()-st_ms) >= TIMEOUT_STREAMDATA ) {
			logd("err-time over [10sec].\n");
			free(rxbuff);
			return -1;
		}

		if ( rxdataframecrc != framecrc ){
			pktsize = pktMakeTxData( PKT_CMD_RESEND, NULL, 0, 0 );
			commWrite( pktGetUartTxData(), pktsize );
			logd( "request resend\n");
		}
		else{
			pktsize = pktMakeTxData( PKT_CMD_TRANSFER_DATA|PKT_REQUEST_OK, NULL, 0, 0 );
			commWrite( pktGetUartTxData(), pktsize );
			memcpy( &data[dataTlen], &rxbuff[4], rxdataframesize);
			dataTlen += rxdataframesize;
			logd("request next [%d]\n", dataTlen );
		}
		
		if ( dataTlen >= info->size ){
			logd("load complete [%d]\n", dataTlen);
			break;	
		}
		rxpktTlen = 0;
	}while(1);

	pktsize = pktMakeTxData( PKT_CMD_TRANSFER_COMPLETE|PKT_REQUEST_OK, NULL, 0, 0 );
	if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
	}
	
	TIME_CHECK_EN();
			
	return info->size;
}


/****************************************
*
*/

int netRecvCamData( char* savepath )
{
	unsigned char* prxdata;
	int rxdatalen;

	int pktsize;
	unsigned short cmd;
	int incv;
	int bytesPerSize;

	// for cam
	char* imgbuff;
	int imgTlen;
	int imgsize;
	int imgwidth;
	int imgheight;
	int imgcrc;
	int rxdataframecrc;
	int framecrc;
	int rxdataframesize;
	char* rxbuff;
	int rxpktTlen;
	int rxpktlen;
	unsigned long long st_ms;

	TIME_CHECK_ST();
	commClearBuff();
	pktsize = pktMakeTxData( PKT_CMD_RECV_CAM_INFO, NULL, 0, 0 );
	if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
		if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
			cmd = pktGetUartRxCmd();
			if ( (unsigned short)cmd == (unsigned short)(PKT_CMD_RECV_CAM_INFO|PKT_REQUEST_OK) ){
				incv=0;
				prxdata = pktGetUartRxData();
				rxdatalen = pktGetUartRxDatalen();
				if ( rxdatalen <= 0 ){
					logd("Rx data 0 \n");
					return -1;
				}
				// receive info
				imgsize = ((prxdata[incv+0]<<24) & 0xff000000) | 
						((prxdata[incv+1]<<16) & 0xff0000) | 
						((prxdata[incv+2]<<8) & 0xff00) | 
						((prxdata[incv+3]<<0) & 0xff) ; 
				incv+=4;
				imgwidth = (short)( ((prxdata[incv+0]<<8) & 0xff00) |
						((prxdata[incv+1]<<0) & 0xff)) ;
				incv+=2;
				imgheight = (short)( ((prxdata[incv+0]<<8) & 0xff00) |
						((prxdata[incv+1]<<0) & 0xff)) ;
				incv+=2;
				bytesPerSize = ((prxdata[incv+0]<<24) & 0xff000000) | 
						((prxdata[incv+1]<<16) & 0xff0000) |
						((prxdata[incv+2]<<8) & 0xff00) |
						((prxdata[incv+3]<<0) & 0xff) ;
				incv+=4;								
				imgcrc = ((prxdata[incv+0]<<24) & 0xff000000) | 
						((prxdata[incv+1]<<16) & 0xff0000) |
						((prxdata[incv+2]<<8) & 0xff00) |
						((prxdata[incv+3]<<0) & 0xff) ;
				
				if( imgwidth == 0 || imgheight == 0 ){
					logd("image width, height 0 \n");
					return -1;
				}
				logp(" img size : %d - width : %d - img height : %d - persize : %d\n",
					imgsize, imgwidth, imgheight, bytesPerSize);

				// receive data
				st_ms = getTimems();
				rxbuff = malloc (UART_MAX_RX_BUFF);
				imgbuff = malloc (UART_MAX_IMGBUFF_RX_BUFF);
				imgTlen = 0;
				rxpktTlen = 0;
				commClearBuff();
				pktsize = pktMakeTxData( PKT_CMD_TRANSFER_READY, NULL, 0, 0 );
				if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
					do{
						while ( (rxpktlen=commRead(&rxbuff[rxpktTlen])) <= 0 ){
							if ( (getTimems()-st_ms) >= TIMEOUT_STREAMDATA ) {
								logd("err-time over [10sec].\n");
								free(rxbuff);
								free(imgbuff);
								return -1;
							}
						};
						
						rxpktTlen += rxpktlen;
						if ( rxpktTlen < 4 ){
							continue;
						}

						printf(" 0x%x 0x%x 0x%x 0x%x \n", rxbuff[0],rxbuff[1],rxbuff[2],rxbuff[3]);
						rxdataframesize = ((rxbuff[0]<<24) & 0xff000000) | 
								((rxbuff[1]<<16) & 0xff0000) |
								((rxbuff[2]<<8) & 0xff00) |
								((rxbuff[3]<<0) & 0xff) ;
						logd("0rx datasize: %d - framesize : %d \n", rxpktTlen, rxdataframesize);
						//load full data
						while ( rxpktTlen < rxdataframesize+8 ){
							if ( (rxpktlen=commRead(&rxbuff[rxpktTlen])) > 0 ){
								rxpktTlen += rxpktlen;
							}
							else if ( (getTimems()-st_ms) >= TIMEOUT_STREAMDATA ) {
								logd("err-time over [10sec].\n");
								free(rxbuff);
								free(imgbuff);
								return -1;
							}
						};

						rxdataframecrc = ((rxbuff[rxdataframesize+4]<<24) & 0xff000000) | 
									((rxbuff[rxdataframesize+5]<<16) & 0xff0000) |
									((rxbuff[rxdataframesize+6]<<8) & 0xff00) |
									((rxbuff[rxdataframesize+7]<<0) & 0xff) ;
						framecrc = pktGetCrc32( &rxbuff[4], rxdataframesize);
						logd("1rx datasize : %d - framesize : %d - crc[0x%x] : 0x%x\n", rxpktTlen, rxdataframesize, framecrc, rxdataframecrc);
						commClearBuff();

						
						if ( (getTimems()-st_ms) >= TIMEOUT_STREAMDATA ) {
							logd("err-time over [10sec].\n");
							free(rxbuff);
							free(imgbuff);
							return -1;
						}

						if ( rxdataframecrc != framecrc ){
							pktsize = pktMakeTxData( PKT_CMD_RESEND, NULL, 0, 0 );
							commWrite( pktGetUartTxData(), pktsize );
							logd( "request resend\n");
						}
						else{
							pktsize = pktMakeTxData( PKT_CMD_TRANSFER_DATA|PKT_REQUEST_OK, NULL, 0, 0 );
							commWrite( pktGetUartTxData(), pktsize );
							memcpy( &imgbuff[imgTlen], &rxbuff[4], rxdataframesize);
							imgTlen += rxdataframesize;
							logd("request next [%d]\n", imgTlen );
						}
						
						if ( imgTlen >= imgsize ){
							logd("load complete [%d]\n", imgTlen);
							break;	
						}

						rxpktTlen = 0;
					}while(1);
					
					if ( savepath != NULL ){
						logp("write file [%s]\n", savepath );
						exfFileOut( savepath, imgbuff, imgTlen );
					}
										
					pktsize = pktMakeTxData( PKT_CMD_TRANSFER_COMPLETE|PKT_REQUEST_OK, NULL, 0, 0 );
					if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
						
					}
				}
			}
			else{
				logp("cam info fail\n");
			}
		}
		else{
			logd("send fail cam\n");
		}
	}
	TIME_CHECK_EN();
}
