/*
  comm.c - linux sample code for voyager
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <termios.h>                 
#include <fcntl.h>                  
#include <pthread.h>
#include <time.h>

#include "app.h"

static int sg_comm_fd;
static int sg_comm_baud;
static int sg_comm_frame_length;

#define MAX_COMM_WAIT_TIME	(1000)
#define MAX_COMM_BUFF	(1024*1024*1)
#define MAX_LINUX_COMM_BUFF	(4096)
static char* sg_comm_buff;

static pthread_mutex_t sg_commmutex_lock   = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   sg_commthread_cond  = PTHREAD_COND_INITIALIZER;


#define COMM_QUE_SIZE	(4096)
char* sg_comm_queue;
int sg_comm_queue_header;
int sg_comm_queue_tail;

// simple queue - linear
int commInitQueue(int size)
{
	sg_comm_queue = malloc(size);
	sg_comm_queue_header = 0;
	sg_comm_queue_tail = 0;
}

int commCloseQueue()
{
	free(sg_comm_queue);
}

int commDeQueue()
{
	if ( sg_comm_queue_header >= sg_comm_queue_tail )
		return -1000;
		
	return (int)sg_comm_queue[sg_comm_queue_header++];
}

void commEnQueue( char indata)
{
	sg_comm_queue[sg_comm_queue_tail++] = indata;
}

void commQueueClear()
{
	memset(sg_comm_queue, 0, COMM_QUE_SIZE);
	sg_comm_queue_header = 0;
	sg_comm_queue_tail = 0;
}

int commGetQueueCount()
{
	return sg_comm_queue_tail - sg_comm_queue_header;
}

int commQueueCopyto(char* dstdata, int offset)
{
	int qlen = sg_comm_queue_tail - sg_comm_queue_header;
	
	if ( qlen == 0 )
		return 0;
	
	memcpy(dstdata, &sg_comm_queue[sg_comm_queue_header+offset], qlen - offset);
	
	return qlen - offset;
}


int commGetLinkSpeed()
{
	return sg_comm_baud;
}

void commSetFrameLength( int size )
{
	sg_comm_frame_length = size;
}

int commGetFrameLength()
{
	return sg_comm_frame_length;
}

int commWriteByte( char b )
{
	if( sg_comm_fd != 0 ){
		while(1){
			if( write( sg_comm_fd, &b, 1 ) > 0 )
				break;
		}
		return 1;
	}

	return 0;
}

int commWrite( char* data, int len )
{
	int i;
	if( sg_comm_fd != 0 ){
		for(i=0;i<len;i++){
			commWriteByte( data[i] );
			//printf("0x%02x ",data[i]);
		}
		return len;
	}
	return 0;
}

int commRead( char* data )
{
	int poll_state;
	int read_count=0;
	struct pollfd     poll_events;  

	poll_events.fd        = sg_comm_fd;
	poll_events.events    = POLLIN | POLLERR;         
	poll_events.revents   = 0;

	if( sg_comm_fd != 0 ){
		poll_state = poll( (struct pollfd*)&poll_events, 1, 20 );
		if ( 0 < poll_state){
			if ( poll_events.revents & POLLIN){
				read_count = read( sg_comm_fd, data, MAX_COMM_BUFF);
			}
			if ( poll_events.revents & POLLERR){
				logd( "uart com error.\n");
			}		
		}
	}
	
	return read_count;
}

int commReadQueue()
{
	int poll_state;
	int read_count=0;
	struct pollfd     poll_events;  
	char readbyte[32];
	
	int rr;
	
	poll_events.fd        = sg_comm_fd;
	poll_events.events    = POLLIN | POLLERR;         
	poll_events.revents   = 0;

	if( sg_comm_fd != 0 ){
		poll_state = poll( (struct pollfd*)&poll_events, 1, 30 );
		if ( 0 < poll_state){
			if ( poll_events.revents & POLLIN){
				rr = read( sg_comm_fd, readbyte, 1);
				commEnQueue(readbyte[0]);
				++read_count;
			}
			if ( poll_events.revents & POLLERR){
				logd( "uart com error.\n");
			}		
		}
	}
	
	return read_count;
}

void commClearBuff()
{
	char empty[256];
	while( read( sg_comm_fd, empty, 256) > 0 );
	commQueueClear();
}

char* commGetRecvPktData()
{
	return sg_comm_buff;
}

int commRecvPktData( int timeout )
{
#if 0	
	unsigned long long st_ms;
	int recvbytes;
	int recvTotalBytes = 0;

	st_ms = getTimems();
	do{
		recvbytes = commRead( &sg_comm_buff[recvTotalBytes] );
		if ( recvbytes <= 0){
			if ( (getTimems()-st_ms) > timeout )
			{
				logd("not enough input data  %d\n", recvTotalBytes );
				pktParseRxData( NULL );
				return -1;
			}
		}
		recvTotalBytes += recvbytes;
		if ( recvTotalBytes > 3 ){
			if ( (short)recvTotalBytes >= (short)pktParseRxPktLen(sg_comm_buff) ){
				break;
			}
		}
	}while (1);
	//logd("commRecvPktData   %d / %d\n", recvTotalBytes , pktParseRxPktLen(sg_comm_buff) );
	pktParseRxData( sg_comm_buff );
	return recvTotalBytes;
#else  // queue mode
	return commRecvPktDataQueue(timeout);
#endif	
}

int commRecvPktDataQueue( int timeout )
{
	unsigned long st_ms;
	int recvbytes;
	int recvTotalBytes = 0;

	int i;
	int plen;
	
	st_ms = getTimems();
	recvTotalBytes = commGetQueueCount();
	
	do{
		recvbytes = commReadQueue();
		if ( recvbytes <= 0){
			if ( (getTimems()-st_ms) > timeout )
			{
				logd("not enough input data  %d\n", recvTotalBytes );
				pktParseRxData( NULL );
				return -1;
			}
		}
			
		recvTotalBytes += recvbytes;
		if ( recvTotalBytes > 3 ){
			commQueueCopyto(sg_comm_buff, 0);
			plen = pktParseRxPktLen(sg_comm_buff);
			if ( recvTotalBytes >= plen ){
				break;
			}
		}
	}while (1);
	//logd("commRecvPktData   %d / %d\n", recvTotalBytes , pktParseRxPktLen(sg_comm_buff) );
	//commQueueCopyto(sg_comm_buff, 0);	
	
	for( i=0; i <plen; i++ ){
		sg_comm_buff[i] = commDeQueue();
	}

	sg_comm_buff[i] = 0;
	
	pktParseRxData( sg_comm_buff );
	return recvTotalBytes;
}

int commChangeBaudAuto( int baud )
{
	int ret;
	char   buf[8];
	struct termios    newtio;
	unsigned short pktcmd;
	int i;
	int cflagcount;

	unsigned long long st_ms;
	char rxbuf[256];
	int recvbytes;
	int recvTotalBytes = 0;

	unsigned int cflag[]={
		(B3000000 | CS8 | CLOCAL | CREAD), (B2500000 | CS8 | CLOCAL | CREAD), (B2000000 | CS8 | CLOCAL | CREAD),
		(B1500000 | CS8 | CLOCAL | CREAD), (B1000000 | CS8 | CLOCAL | CREAD), (B921600 | CS8 | CLOCAL | CREAD), 
		(B500000 | CS8 | CLOCAL | CREAD), (B460800 | CS8 | CLOCAL | CREAD), (B230400 | CS8 | CLOCAL | CREAD),   
		(B115200 | CS8 | CLOCAL | CREAD), (B57600 | CS8 | CLOCAL | CREAD), (B9600 | CS8 | CLOCAL | CREAD), 
	};
	
	cflagcount = sizeof(cflag) >> 2;

	for( i=0; i< cflagcount; i++ ){
		// set baud
		memset( &newtio, 0, sizeof(newtio) );
		newtio.c_cflag = cflag[i];
		newtio.c_oflag       = 0;
		newtio.c_lflag       = 0;
		newtio.c_cc[VTIME]   = 0;
		newtio.c_cc[VMIN]    = 1;
		tcflush  (sg_comm_fd, TCIFLUSH );
		tcsetattr(sg_comm_fd, TCSANOW, &newtio );
		fcntl(sg_comm_fd, F_SETFL, FNDELAY); 
		usleep(50*1000); // 
		commClearBuff();
		usleep(50*1000); // 
		ret = pktMakeTxData( PKT_CMD_TEST, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), ret ) > 0 ){
			st_ms = getTimems();
			recvTotalBytes = 0;
			memset( rxbuf, 0 , 256 );
			do{
				recvbytes = commRead( &rxbuf[recvTotalBytes] );
				recvTotalBytes += recvbytes;
				if ( recvTotalBytes > 3 ){
					if ( (short)recvTotalBytes >= (short)pktParseRxPktLen(rxbuf) ){
						break;
					}
				}
			}while ( getTimems()-st_ms < 300 );
			
			if ( getTimems()-st_ms > 300 ){
				logp("test fail [%d] - time over\n", i);
			}
			else{
				pktParseRxData( rxbuf );
				pktcmd = pktGetUartRxCmd();
				if ( pktcmd == (unsigned short)(PKT_CMD_TEST|PKT_REQUEST_OK) ){
					logp("test ok [%d]\n", i);
					break;
				}
				else{
					logp("test fail [%d]\n", i);
				}
			}
		}
	}

	if ( i== cflagcount){
		logd("not support baud(voyager)\n");
		return -1;
	}

	buf[0] = (baud >> 24) &0xff;
	buf[1] = (baud >> 16) &0xff;
	buf[2] = (baud >> 8) &0xff;
	buf[3] = (baud >> 0) &0xff;
	ret = pktMakeTxData( PKT_CMD_SET_BAUD, buf, 4, 0 );
	commWrite( pktGetUartTxData(), ret );
	usleep(50000); // 

	// set baud
	memset( &newtio, 0, sizeof(newtio) );
	if (baud == 9600 ){
	   newtio.c_cflag       = B9600 | CS8 | CLOCAL | CREAD;
	}else if(baud==57600){
	   newtio.c_cflag       = B115200 | CS8 | CLOCAL | CREAD;
	}else if(baud==115200){
	   newtio.c_cflag       = B115200 | CS8 | CLOCAL | CREAD;
	}else if(baud==230400){
	   newtio.c_cflag       = B230400 | CS8 | CLOCAL | CREAD;
	}else if(baud==460800){
	   newtio.c_cflag       = B460800 | CS8 | CLOCAL | CREAD;
	}else if(baud==500000){
	   newtio.c_cflag       = B500000 | CS8 | CLOCAL | CREAD;
	}else if(baud==921600){
	   newtio.c_cflag       = B921600 | CS8 | CLOCAL | CREAD;
	}else if(baud==1000000){
	   newtio.c_cflag       = B1000000 | CS8 | CLOCAL | CREAD;
	}else if(baud==1500000){
	   newtio.c_cflag       = B1500000 | CS8 | CLOCAL | CREAD;
	}else if(baud==2000000){
	   newtio.c_cflag       = B2000000 | CS8 | CLOCAL | CREAD;
	}else if(baud==2500000){
	   newtio.c_cflag       = B2500000 | CS8 | CLOCAL | CREAD;
	}else if(baud==3000000){
	   newtio.c_cflag       = B3000000 | CS8 | CLOCAL | CREAD;
	}else{
		logd("not support baud\n");
		return -1;
	}

	logp("link speed : %d \n", baud);
	sg_comm_baud = baud;
	newtio.c_oflag       = 0;
	newtio.c_lflag       = 0;
	newtio.c_cc[VTIME]   = 0;
	newtio.c_cc[VMIN]    = 1;
	tcflush  (sg_comm_fd, TCIFLUSH );
	tcsetattr(sg_comm_fd, TCSANOW, &newtio );
	fcntl(sg_comm_fd, F_SETFL, FNDELAY);
	usleep(50000); // 50ms
	
	sg_comm_frame_length = sg_comm_baud>>3;  // default frame length
	#if 0
	buf[0] = (sg_comm_frame_length >> 24) &0xff;
	buf[1] = (sg_comm_frame_length >> 16) &0xff;
	buf[2] = (sg_comm_frame_length >> 8) &0xff;
	buf[3] = (sg_comm_frame_length >> 0) &0xff;
	ret = pktMakeTxData( PKT_CMD_SET_PKT_FRAME_LENGTH, buf, 4, 0 );
	commWrite( pktGetUartTxData(), ret );
	commRecvPktData( TIMEOUT_DEFAULT_RECVDATA );
	#endif
	return 0;
}

int commChangeBaud( int prevbaud, int chbaud)
{
	int ret;
	char   buf[8];
	struct termios    newtio;

	// set baud
	memset( &newtio, 0, sizeof(newtio) );
	if (prevbaud == 9600 ){
	   newtio.c_cflag       = B9600 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==57600){
	   newtio.c_cflag       = B115200 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==115200){
	   newtio.c_cflag       = B115200 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==230400){
	   newtio.c_cflag       = B230400 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==460800){
	   newtio.c_cflag       = B460800 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==500000){
	   newtio.c_cflag       = B500000 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==921600){
	   newtio.c_cflag       = B921600 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==1000000){
	   newtio.c_cflag       = B1000000 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==1500000){
	   newtio.c_cflag       = B1500000 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==2000000){
	   newtio.c_cflag       = B2000000 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==2500000){
	   newtio.c_cflag       = B2500000 | CS8 | CLOCAL | CREAD;
	}else if(prevbaud==3000000){
	   newtio.c_cflag       = B3000000 | CS8 | CLOCAL | CREAD;
	}else{
		logd("not support baud\n");
		return -1;
	}
	newtio.c_oflag       = 0;
	newtio.c_lflag       = 0;
	newtio.c_cc[VTIME]   = 0;
	newtio.c_cc[VMIN]    = 1;
	tcflush  (sg_comm_fd, TCIFLUSH );
	tcsetattr(sg_comm_fd, TCSANOW, &newtio );
	fcntl(sg_comm_fd, F_SETFL, FNDELAY); 
	
	usleep(50000); // 50ms

	buf[0] = (chbaud >> 24) &0xff;
	buf[1] = (chbaud >> 16) &0xff;
	buf[2] = (chbaud >> 8) &0xff;
	buf[3] = (chbaud >> 0) &0xff;
	ret = pktMakeTxData( PKT_CMD_SET_BAUD, buf, 4, 0 );
	commWrite( pktGetUartTxData(), ret );
	usleep(50000); // 50ms

	// set baud
	memset( &newtio, 0, sizeof(newtio) );
	if (chbaud == 9600 ){
	   newtio.c_cflag       = B9600 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==57600){
	   newtio.c_cflag       = B115200 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==115200){
	   newtio.c_cflag       = B115200 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==230400){
	   newtio.c_cflag       = B230400 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==460800){
	   newtio.c_cflag       = B460800 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==500000){
	   newtio.c_cflag       = B500000 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==921600){
	   newtio.c_cflag       = B921600 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==1000000){
	   newtio.c_cflag       = B1000000 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==1500000){
	   newtio.c_cflag       = B1500000 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==2000000){
	   newtio.c_cflag       = B2000000 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==2500000){
	   newtio.c_cflag       = B2500000 | CS8 | CLOCAL | CREAD;
	}else if(chbaud==3000000){
	   newtio.c_cflag       = B3000000 | CS8 | CLOCAL | CREAD;
	}else{
		logd("not support baud\n");
		return -1;
	}
	logp("link speed : %d \n", chbaud);
	sg_comm_baud = chbaud;
	
	newtio.c_oflag       = 0;
	newtio.c_lflag       = 0;
	newtio.c_cc[VTIME]   = 0;
	newtio.c_cc[VMIN]    = 1;

	tcflush  (sg_comm_fd, TCIFLUSH );
	tcsetattr(sg_comm_fd, TCSANOW, &newtio );
	fcntl(sg_comm_fd, F_SETFL, FNDELAY);
	usleep(50000); // 50ms
	
	sg_comm_frame_length = sg_comm_baud>>3;  // default frame length
#if 0
	buf[0] = (sg_comm_frame_length >> 24) &0xff;
	buf[1] = (sg_comm_frame_length >> 16) &0xff;
	buf[2] = (sg_comm_frame_length >> 8) &0xff;
	buf[3] = (sg_comm_frame_length >> 0) &0xff;
	ret = pktMakeTxData( PKT_CMD_SET_PKT_FRAME_LENGTH, buf, 4, 0 );
	commWrite( pktGetUartTxData(), ret );
	commRecvPktData( TIMEOUT_DEFAULT_RECVDATA );
#endif	
	
	return 0;
}

int commOpen( char* dev, int baud )
{
	int ret;

	sg_comm_buff = malloc( MAX_COMM_BUFF );		
	commInitQueue(COMM_QUE_SIZE);
		
	sg_comm_fd = open( dev, O_RDWR | O_NOCTTY | O_NONBLOCK );     
	if ( 0 > sg_comm_fd){        
		logd("[%s] open error\n", dev);
		free(	sg_comm_buff );
		return -1;
	}
	//commChangeBaud( 9600, baud );
	if ( commChangeBaudAuto(baud) < 0 ){
		close( sg_comm_fd );
		free(	sg_comm_buff );
		return -1;
	}

	return 0;
}

int commClose()
{
	if ( sg_comm_fd != 0){
		close ( sg_comm_fd );
	}
	sg_comm_fd = 0;
	commCloseQueue();
}

