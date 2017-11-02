/*
  cli.c - linux sample code for voyager
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

const char* FIFO_FACE_DETECT = "face_detect_fifo";

int fd_fifo;

typedef struct _cmd_struct{
    char name[16];
	char usage[80];
	char desc[512];
	void (*func)(void* _param);
}CmdStruct ;

#define MAX_CMD_BUFF		256
#define MAX_CMD_ARGC		15

/*
* setup version
*/
//static char sg_app_version[]="v0.0.1";

/*
* added - baud, version, frame length
* release first version
*/
//static char sg_app_version[]="v0.1.2";

/*
* cmd set - compatible voyclient
* release stable version
*/
static char sg_app_version[]="v0.2.0";

static char sg_cmd_buf[MAX_CMD_BUFF];
static char sg_cmd_param[MAX_CMD_ARGC][MAX_CMD_BUFF];

void cli_do_version(void* _param);
void cli_do_help(void* _param);
void cli_do_sys(void* _param);
void cli_do_uf(void* _param);
void cli_do_cam(void* _param);
void cli_do_test(void* _param);

CmdStruct sg_cmd_list[]=
{
	{"help", "help", "@display cmd list.", cli_do_help},
	{"version", "version", "@display app version.", cli_do_version},
	{"sys", "sys [sub]", "@system control\n" 
			"  [reboot] -voyager reboot\n"
			"  [test] -com test\n"
			"  [alive] -is alive?\n"
			"  [time] -set target time(ms)\n"
			"  [login] [PW] - login voyager\n"
			"  [logout] [PW] - logout voyager\n"
			"  [setpw] [prev-PW] [new-PW] -set conection password\n"
			"  [baud] [baudrate] -set baudreate\n"
			"  [flen] [size] -set frame length\n"
			"  [version] -display voyager version\n"
			"  [update] [filename] -app update\n", cli_do_sys},
	{"uf", "uf [sub]", "@uerface control\n"
			"  [init] -uerface initialize\n"
			"  [status]-uerface status\n"
			"  [reset] -uerface db reset\n"
			"  [reg] -uerface registration\n"
			"  [rec] -uerface recognition\n"
			"  [recf] -uerface recognition : freerunmode \n"
			"  [userlist] -uerface user list\n"
			"  [userdel] -uerface user delete\n"
			"  [seclv] -uerface set security level\n"
			"  [udata] [get/enroll] [ID(get:id,en:filename] -Template Data Transfer \n" 
			"  [fd] [loop-count]-uerface face detetion\n", cli_do_uf},
	{"cam", "cam [sub]", "@camera control\n"
			"  [init] -init camera\n"
			"  [led]-led toggle\n"
			"  [close] -close camera\n"
			"  [read] [filename] - read camera data\n", cli_do_cam},
	{"test", "test [sub]", "@test mode\n", cli_do_test},
};

int getFaceDetectFifoDescriptor(){
	int fd = -1;
	while(1){
		printf("[alert] %s\n","Jump in to the loop");
		if((fd = open(FIFO_FACE_DETECT, O_RDWR|O_NONBLOCK)) == -1){
			printf("[alert] %s\n","There is some problem");
			if(errno == ENOENT){
				printf("[alert] %s\n","Fifo which you selected doesn't exist on your directory");
				if(mkfifo(FIFO_FACE_DETECT, 0644) == -1){
					printf("[alert] %s\n","Failed to make fifo");
				}
				else printf("[alert] %s\n","Fifo is made sucessfully");
				continue;
			}
			perror("open");
			exit(1);
		}
		else return fd;
	}
}

int cli_cmd_parser(char *cmd)
{
	int i;
	int cmd_cnt;
	int head_idx;
	
	char cmdbuf[256];

	if(*cmd == '\0')
		return -1;

	memset ( sg_cmd_param, 0, sizeof(sg_cmd_param) );
	memset( cmdbuf, 0, 256 );
	strncpy( cmdbuf, cmd, strlen(cmd) );

	cmd_cnt = 0;
	head_idx = 0;
	for(i=0 ; cmdbuf[i] != '\0' ; i++){
		if(cmdbuf[i] != ' ')
			continue;
		else
			cmdbuf[i]='\0';
		
		if( head_idx != i ){			
			strcpy(sg_cmd_param[cmd_cnt], &cmdbuf[head_idx] );
			++cmd_cnt;
		}
		
		head_idx = i+1;
	}

	if( head_idx != i ){			
		strcpy(sg_cmd_param[cmd_cnt], &cmdbuf[head_idx] );
		++cmd_cnt;
	}

	return cmd_cnt;
}

int cli_check_cmd(char *cmd_name)
{
	int i;

	for(i=0;i<sizeof(sg_cmd_list)/sizeof(CmdStruct);i++)
		if(!strcmp(sg_cmd_list[i].name, cmd_name))
			return i;

	return -1;
}

/**************************************************************
*
*/
void cli_do_version(void* _param)
{
	logp("\napp version : %s\n\n", sg_app_version);
}

/**************************************************************
*
*/
void cli_do_help(void* _param)
{
	int i;
	logp("\n>>cmd list<<\n");
	for(i=0;i<sizeof(sg_cmd_list)/sizeof(CmdStruct);i++){
		logp(".%-12s\t",sg_cmd_list[i].name);
		logp("%s\n\r",sg_cmd_list[i].usage);
		logp("  %s\n\r", sg_cmd_list[i].desc);
	}
	logp(".%-12s\t","exit");
	logp("%s\n\r","app exit");
	logp("  %s\n\r", "exit");
}

/**************************************************************
*
*/
void cli_do_sys(void* _param)
{
	unsigned char* prxdata;
	unsigned char* ptxdata;
	int rxdatalen;
	int txdatalen;

	unsigned char strbuf[256];
	int strbuflen;

	unsigned char prxbuff[2048];
	unsigned char ptxbuff[2048];
	int rxbuffalen;
	int txbufflen;

	int pktsize;
	unsigned short cmd;
	int incv;
	unsigned long long st_ms;

	int ret;
	int loopcnt;
	int loop0, loop1;
	int trycount;

	int intvar0;

	if ( sg_cmd_param[1][0] == 0 ){
		logp("not supported.\n");
		return;
	}

	if( !strcmp(sg_cmd_param[1], "test") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_TEST, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_TEST|PKT_REQUEST_OK) ){
					logp("test ok\n");
				}
				else{
					logp("test fail\n");
				}
			}
			else{
				logd("send fail test\n");
			}
		}
	}
	else if( !strcmp(sg_cmd_param[1], "freset") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_FACTRESET, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 20000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_FACTRESET|PKT_REQUEST_OK) ){
					logp("freset ok\n");
				}
				else{
					logp("freset fail\n");
				}
			}
			else{
				logd("send fail freset\n");
			}
		}
	}
	else if( !strcmp(sg_cmd_param[1], "update") ){
		if ( sg_cmd_param[2][0] == 0 ){
			logd("input update file path\n");
			return;
		}

		txdatalen = exfFileSize( sg_cmd_param[2] );
		if ( txdatalen <= 0 ){
			logd("check file or path \n");
			return;
		}

		ptxdata  = malloc ( txdatalen );
		exfFileIn( sg_cmd_param[2], ptxdata, txdatalen );
		TIME_CHECK_ST();
		ret = netSendData( ptxdata, txdatalen, 0, 0 ); 
		TIME_CHECK_EN();
		if ( ret > 0 ){
			logp("send file complete\n");
			logp("update app (y:yes, n:no) ?");
			memset(strbuf, 0, 256);
			fgets(strbuf, 256, stdin);
			fflush(stdin);
			fflush(stdout);

			strbuflen = strlen(strbuf)-1;
			strbuf[strbuflen] = '\0';
			
			if ( strbuf[0] == 'y' || strbuf[0]=='Y' ){
				commClearBuff();
				pktsize = pktMakeTxData( PKT_CMD_UPDATE_APP, NULL, 0, 0 );
				if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
					if ( commRecvPktData( 10000 ) > 0 ){
						cmd = pktGetUartRxCmd();
						if ( cmd == (unsigned short)(PKT_CMD_UPDATE_APP|PKT_REQUEST_OK) ){
							logp("update ok\n");
						}else{
							logp("update fail\n");
						}
					}
					else{
						logd("recv fail.\n");
					}
				}				
				else{
					logd("send fail update.\n");
				}
			}
			else{
				logp("update is canceled.\n");
			}
		}
		else{
			logd("send fail data.\n");
		}
		free ( ptxdata );
	}

	else if( !strcmp(sg_cmd_param[1], "login") ){
		if( sg_cmd_param[2][0] == 0 ){
			logp("error, plz input pw [sys login pw]\n");	
			return;
		}
		
		strbuflen = strlen(sg_cmd_param[2]);

		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_LOGIN_USER, sg_cmd_param[2], strbuflen, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_LOGIN_USER|PKT_REQUEST_OK) ){
					logp("voyager login success.\n");
				}
				else{
					logp("plz. check password.\n");
				}
			}
			else{
				logd("recv fail.\n");
			}
		}
	}
	
	else if( !strcmp(sg_cmd_param[1], "logout") ){
		if( sg_cmd_param[2][0] == 0 ){
			logp("error, plz input pw [sys login pw]\n");	
			return;
		}
		
		strbuflen = strlen(sg_cmd_param[2]);
		
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_LOGOUT_USER, sg_cmd_param[2], strbuflen, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_LOGOUT_USER|PKT_REQUEST_OK) ){
					logp("voyager logout success.\n");
				}
				else{
					logp("plz. check password.\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}
	
	else if( !strcmp(sg_cmd_param[1], "setpw") ){
		if( sg_cmd_param[2][0] == 0 || sg_cmd_param[3][0] == 0 ){
			logp("error. plz, setpw [prev-PW] [new-PW]\n");	
			return;
		}

		memset(strbuf, 0, 256);
		strbuflen = strlen(sg_cmd_param[2]);
		memcpy( strbuf, sg_cmd_param[2], strbuflen);
				
		strbuflen = strlen(sg_cmd_param[3]);
		memcpy( &strbuf[64], sg_cmd_param[3], strbuflen);
		
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_SET_USER_PW, strbuf, 128, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 10000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_SET_USER_PW|PKT_REQUEST_OK) ){
					logp("password was changed.\n");
				}
				else{
					logp("plz. check prev password.\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}
	
	else if( !strcmp(sg_cmd_param[1], "alive") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_ALIVE, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_ALIVE|PKT_REQUEST_OK) ){
					logp("alive\n");
				}
				else{
					logp("dead\n");
				}
			}
			else{
				logd("send fail alive\n");
			}
		}
	}

	else if( !strcmp(sg_cmd_param[1], "reboot") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_REBOOT, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			usleep(500*1000); 
			commChangeBaud(	UART_DEFAULT_BAUDRATE, commGetLinkSpeed() );
			logd("reboot ok.\n");
		}
		else{
			logd("send fail reboot\n");
		}
	}

	else if( !strcmp(sg_cmd_param[1], "time") ){
		commClearBuff();
		st_ms = getTimems();
		ptxbuff[0] = (unsigned char)( (st_ms>>56)&0xff );
		ptxbuff[1] = (unsigned char)( (st_ms>>48)&0xff );
		ptxbuff[2] = (unsigned char)( (st_ms>>40)&0xff );
		ptxbuff[3] = (unsigned char)( (st_ms>>32)&0xff );
		ptxbuff[4] = (unsigned char)( (st_ms>>24)&0xff );
		ptxbuff[5] = (unsigned char)( (st_ms>>16)&0xff );
		ptxbuff[6] = (unsigned char)( (st_ms>>8)&0xff );
		ptxbuff[7] = (unsigned char)( (st_ms>>0)&0xff );
		pktsize = pktMakeTxData( PKT_CMD_SET_SYSTIME, ptxbuff, 8, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_SET_SYSTIME|PKT_REQUEST_OK) ){
					logp("time set ok\n");
				}
				else{
					logp("time set fail\n");
				}
			}
			else{
				logd("send fail timeset\n");
			}
		}
	}
	else if( !strcmp(sg_cmd_param[1], "flen") ){
		if ( sg_cmd_param[2][0] != 0 ){
			intvar0 = atoi(sg_cmd_param[2]);
		}
		else{
			return;
		}
		
		intvar0 >>= 3;
		ptxbuff[0] = (unsigned char)( (intvar0>>24)&0xff );
		ptxbuff[1] = (unsigned char)( (intvar0>>16)&0xff );
		ptxbuff[2] = (unsigned char)( (intvar0>>8)&0xff );
		ptxbuff[3] = (unsigned char)( (intvar0>>0)&0xff );
		
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_SET_PKT_FRAME_LENGTH, ptxbuff, 4, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_SET_PKT_FRAME_LENGTH|PKT_REQUEST_OK) ){
					logp("frame length %d ok\n", intvar0);
					commSetFrameLength(intvar0);
				}
				else{
					logp("frame length set fail\n");
				}
			}
			else{
				logd("send fail frame length \n");
			}
		}
	}
	else if( !strcmp(sg_cmd_param[1], "baud") ){
		if ( sg_cmd_param[2][0] != 0 ){
			intvar0 = atoi(sg_cmd_param[2]);
		}
		else{
			return;
		}
		
		if ( commChangeBaud( commGetLinkSpeed(), intvar0 ) >= 0 ){
			logd("change baudrate ok\n");
		}
		else{
			logd("error host baud. plz this problem checked\n");
		}
	}
	else if( !strcmp(sg_cmd_param[1], "version") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_GET_APPVER, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_GET_APPVER|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					intvar0 = ((prxdata[0]<<24) & 0xff000000) | 
						((prxdata[1]<<16) & 0xff0000) |
						((prxdata[2]<<8) & 0xff00) |
						((prxdata[3]<<0) & 0xff) ;
					logp("app version %d.%d\n", ((intvar0>>16)&0xffff), ((intvar0>>0)&0xffff));
				}
				else{
					logp("get app version fail\n");
				}
			}
			else{
				logd("send fail get version \n");
			}
		}
		
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_GET_UERFACE_VER, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_GET_UERFACE_VER|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					logp("UerFace version %s\n", prxdata);
				}
				else{
					logp("get UerFace version fail\n");
				}
			}
			else{
				logd("send fail get version \n");
			}
		}
	}	
	else if( !strcmp(sg_cmd_param[1], "seckey") ){
		int i;
    char pktparam[128];
    
    for(i=0;i<32;i++){
        pktparam[i] = (char)(i+5);
    }

    commClearBuff();
    pktsize = pktMakeTxData( PKT_CMD_SET_SEC_KEY, pktparam, 32, 0 );   
    if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
      if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
      	cmd = pktGetUartRxCmd();	
      	if ( cmd == (unsigned short)(PKT_CMD_SET_SEC_KEY|PKT_REQUEST_OK) ){
					logp("set sec key ok\n");
				}
				else{
					logp("set sec key fail\n");
				}
      }
    }
	}
	else if( !strcmp(sg_cmd_param[1], "temp") ){

	}
	else{
		logp("not supported.\n");
	}
}

/**************************************************************
*
*/
void cli_do_uf(void* _param)
{
	unsigned char txdata[1024];
	unsigned char* prxdata;
	unsigned char* ptxdata;	
	unsigned short cmd;
	int txdatalen;
	int pktsize;
	
	int rxdatalen;
	int retcode;
					
	int incv;
	int ret;
	int loopcnt;
	int loop0, loop1;

	char tmpstr[32];

	if ( sg_cmd_param[1][0] == 0 ){
		logp("not supported.\n");
		return;
	}

	if( !strcmp(sg_cmd_param[1], "init") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_INIT, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_INIT|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();
					incv = 0;
					retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
							((prxdata[incv+1]<<16) & 0xff0000) |
							((prxdata[incv+2]<<8) & 0xff00) |
							((prxdata[incv+3]<<0) & 0xff) ;
					
					if ( retcode >= 0 ){
						logp("uf init success\n");
					}					
					else{
						logp("uf init fail\n");
					}
				}
				else{
					logp("error uf init\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}

	else if( !strcmp(sg_cmd_param[1], "status") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_STATUS, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_STATUS|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();
					incv = 0;
					retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
							((prxdata[incv+1]<<16) & 0xff0000) |
							((prxdata[incv+2]<<8) & 0xff00) |
							((prxdata[incv+3]<<0) & 0xff) ;
					
					if ( retcode >= 0 ){
						logp("uf status ok\n");
					}					
					else{
						logp("uf status fail\n");
					}
				}
				else{
					logp("error uf status\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}

	else if( !strcmp(sg_cmd_param[1], "reset") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_RESET, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 10000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_RESET|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();
					incv = 0;
					retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
							((prxdata[incv+1]<<16) & 0xff0000) |
							((prxdata[incv+2]<<8) & 0xff00) |
							((prxdata[incv+3]<<0) & 0xff) ;
					
					if ( retcode >= 0 ){
						logp("uf reset ok\n");
					}					
					else{
						logp("uf reset fail\n");
					}
				}
				else{
					logp("error uf reset\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}

	else if ( !strcmp(sg_cmd_param[1], "fd") ){

		fd_fifo = getFaceDetectFifoDescriptor();

		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_FACECHECK_INIT, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 10000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_FACECHECK_INIT|PKT_REQUEST_OK) ){
					logp("fd init ok\n");
					
					loopcnt = 1;
					if ( sg_cmd_param[2][0] != 0 ){
						loopcnt = atoi(sg_cmd_param[2]);
					}
					
					for( loop0=0; loop0<loopcnt; loop0++ ){
						ufFaceDetect(fd_fifo);
					}
	if(fd_fifo != -1) close(fd_fifo);
				}
				else{
					logp("fd init fail\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}

	else if ( !strcmp(sg_cmd_param[1], "userlist") ){
		ufLoadUserList();
	}

	else if ( !strcmp(sg_cmd_param[1], "seclv") ){
		logp("input secure level (0~5) : ");
		memset(txdata, 0, 256);
		fgets(txdata, 256, stdin);
		fflush(stdin);
		fflush(stdout);

		txdatalen = strlen(txdata)-1;
		txdata[txdatalen] = '\0';

		if ( txdatalen != 1 ){
			logp("error check input id \n");
			return;
		}

		incv = txdata[0] - '0';
		txdata[0] = (incv >> 24) & 0xff;
		txdata[1] = (incv >> 16) & 0xff;
		txdata[2] = (incv >> 8) & 0xff;
		txdata[3] = (incv >> 0) & 0xff;

		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_SET_SECLEVEL, txdata, 4, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 1000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_SET_SECLEVEL|PKT_REQUEST_OK) ){

					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();
					incv = 0;
					retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
							((prxdata[incv+1]<<16) & 0xff0000) |
							((prxdata[incv+2]<<8) & 0xff00) |
							((prxdata[incv+3]<<0) & 0xff) ;
					
					if ( retcode >= 0 ){
						logp("set secure-level succcess\n");
					}					
					else{
					logp("set secure-level fail\n");
					}
				}
				else{
					logp("error set secure-level\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}
	
	else if ( !strcmp(sg_cmd_param[1], "userdel") ){
		logp("input delete ID(max-20digit) : ");
		memset(txdata, 0, 256);
		fgets(txdata, 256, stdin);
		fflush(stdin);
		fflush(stdout);

		txdatalen = strlen(txdata)-1;
		txdata[txdatalen] = '\0';

		if ( txdatalen > 20){
			logp("error check input id \n");
			return;
		}

		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERDEL, txdata, txdatalen, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 1000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERDEL|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();
					incv = 0;
					retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
							((prxdata[incv+1]<<16) & 0xff0000) |
							((prxdata[incv+2]<<8) & 0xff00) |
							((prxdata[incv+3]<<0) & 0xff) ;
					
					if ( retcode >= 0 ){
						logp("userdel success\n");	
					}					
					else{
					logp("userdel fail\n");
					}
				}
				else{
					logp("error userdel\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}

	else if ( !strcmp(sg_cmd_param[1], "rec") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERCHECK_INIT, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 10000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERCHECK_INIT|PKT_REQUEST_OK) ){
					logp("rec init ok\n");
					ret = ufFaceRec( 10000 );
					if ( ret >= 0 ){
						logp("rec ok\n");
					}
					else{
						logp("rec fail\n");
					}
				}
				else{
					logp("error rec\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}

	else if ( !strcmp(sg_cmd_param[1], "recf") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERCHECK_INIT, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 5000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERCHECK_INIT|PKT_REQUEST_OK) ){
					logp("rec init ok\n");
					ret = ufFaceRecFr( 7000 );
					if ( ret >= 0 ){
						logp("rec ok\n");
					}
					else{
						logp("rec fail\n");
					}
				}
				else{
					logp("error rec\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}
	
	else if ( !strcmp(sg_cmd_param[1], "reg") ){
		logp("input your ID(max-20digit) : ");
		memset(txdata, 0, 256);
		fgets(txdata, 256, stdin);
		fflush(stdin);
		fflush(stdout);

		txdatalen = strlen(txdata)-1;
		txdata[txdatalen] = '\0';

		if ( txdatalen > 20){
			logp("error check input id \n");
			return;
		}

		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERADD_INIT, txdata, txdatalen, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 1000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERADD_INIT|PKT_REQUEST_OK) ){
					logp("reg init ok\n");	
					do{
						// reg1 pre 
						logp("wait for your pose.\n");
						if ( ufFaceRegPre( 10000 ) < 0 ){
							logp("time over : your first pose.\n");
							return;
						}
						logp("start this pose face [start: enter]");
						fgets(tmpstr, 32, stdin);
						fflush(stdin);
						fflush(stdout);

						ret = ufFaceReg( 10000 );
						if ( ret == -110 ){
							// reg2 pre 
							logp("wait for your next pose.[continue: enter]\n");
							fgets(tmpstr, 32, stdin);
							fflush(stdin);
							fflush(stdout);
						}
						else if ( ret >= 0 ){
							logp("reg complete! make a decision user save[0] or cancel[other]. ");
							fgets(tmpstr, 32, stdin);
							fflush(stdin);
							fflush(stdout);
			
							txdata[0] = (ret>>24)&0xff;
							txdata[1] = (ret>>16)&0xff;
							txdata[2] = (ret>>8)&0xff;
							txdata[3] = (ret>>0)&0xff;
			
							if ( tmpstr[0] == '0' ){
								commClearBuff();
								pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERADD_SAVE, txdata, 4, 0 );
								if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
									if ( commRecvPktData( 1000 ) > 0 ){
										cmd = pktGetUartRxCmd();
										if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERADD_SAVE|PKT_REQUEST_OK) ){
											logp("reg ok. user added.\n");
										}
										else{
											logp("reg error\n");
										}
									}
								}
							}
							else{
								commClearBuff();
								pktsize = pktMakeTxData( PKT_CMD_UERFACE_USERADD_ROLLBACK, txdata, 4, 0 );
								if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
									if ( commRecvPktData( 1000 ) > 0 ){
										cmd = pktGetUartRxCmd();
										if ( cmd == (unsigned short)(PKT_CMD_UERFACE_USERADD_ROLLBACK|PKT_REQUEST_OK) ){
											logp("reg canceled. \n");
										}
										else{
											logp("reg error\n");
										}
									}
								}
							}
							
							break;
						}
						else{
							logp("timeover! - plz agin reg process.\n");
							break;
						}						
					}while(1);				
				}
				else{
					logp("error reg\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}
	
	else if ( !strcmp(sg_cmd_param[1], "name") ){
		logp("current name(ID,max-20digit) : ");
		memset(txdata, 0, 256);
		fgets(txdata, 64, stdin);
		fflush(stdin);
		fflush(stdout);

		txdatalen = strlen(txdata)-1;
		if ( txdatalen > 20){
			logp("error check input id \n");
			return;
		}
		logp("new name(ID,max-20digit) : ");
		fgets(txdata+20, 64, stdin);
		fflush(stdin);
		fflush(stdout);

		txdatalen = strlen(txdata+20)-1;
		txdata[txdatalen] = '\0';

		if ( txdatalen > 20){
			logp("error check input id \n");
			return;
		}

		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_UERFACE_UPDATE_USERNAME, txdata, 40, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( 1000 ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( cmd == (unsigned short)(PKT_CMD_UERFACE_UPDATE_USERNAME|PKT_REQUEST_OK) ){
					prxdata = pktGetUartRxData();
					rxdatalen = pktGetUartRxDatalen();
					incv = 0;
					retcode = ((prxdata[incv+0]<<24) & 0xff000000) | 
							((prxdata[incv+1]<<16) & 0xff0000) |
							((prxdata[incv+2]<<8) & 0xff00) |
							((prxdata[incv+3]<<0) & 0xff) ;
					
					if ( retcode >= 0 ){
						logp("change name(ID) success\n");	
					}					
					else{
						logp("change name(ID) fail\n");
					}
				}
				else{
					logp("error change name\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}
	
	else if ( !strcmp(sg_cmd_param[1], "udata") ){
    if (!strcmp(sg_cmd_param[2], "enroll"))
    {
    	char extid[24];
      if (sg_cmd_param[3][0]== 0 )
          return;

			strncpy( extid, sg_cmd_param[3], 24);
			extid[strlen(extid)-4] = '\0';

      txdatalen = exfFileSize( sg_cmd_param[3] );
			if ( txdatalen <= 0 ){
				logd("check file or path \n");
				return;
			}
	
			ptxdata  = malloc ( txdatalen );
			exfFileIn( sg_cmd_param[3], ptxdata, txdatalen );
			TIME_CHECK_ST();
			ret = netSendData( ptxdata, txdatalen, 0, 0 ); 
			TIME_CHECK_EN();
			if ( ret > 0 ){
				commClearBuff();
				pktsize = pktMakeTxData( PKT_CMD_ENROLL_UERFACE_USERDATA, extid, 20, 0 );
				if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
					if ( commRecvPktData( 5000 ) > 0 ){
						cmd = pktGetUartRxCmd();
						if ( cmd == (unsigned short)(PKT_CMD_ENROLL_UERFACE_USERDATA|PKT_REQUEST_OK) ){
							logp("Enroll UserData Ok.\n");
						}
						else{
							logp("Enroll UserData Fail.\n");
						}
					}
					else{
						logd("recv fail\n");
					}
				}
      }
    }
    else if (!strcmp(sg_cmd_param[2], "get"))
    {
			if (sg_cmd_param[3][0]== 0 )
			    return;
			
			char* utemplate;
			
			utemplate = malloc(1024*1024);
			memset( txdata, 0, 32 );
			strncpy(txdata, sg_cmd_param[3], 20 );				
			
			commClearBuff();				
			pktsize = pktMakeTxData( PKT_CMD_GET_UERFACE_USERDATA, txdata, 32, 0 );
			if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			  if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
			  	cmd = pktGetUartRxCmd();	
			  	if ( cmd == (unsigned short)(PKT_CMD_GET_UERFACE_USERDATA|PKT_REQUEST_OK) ){
						prxdata = pktGetUartRxData();
						rxdatalen = pktGetUartRxDatalen();							
						
			      incv = 20;
			      int type = ((int)((prxdata[incv + 0] << 24) & 0xff000000) |
			          (int)((prxdata[incv + 1] << 16) & 0xff0000) |
			          (int)((prxdata[incv + 2] << 8) & 0xff00) |
			          (int)((prxdata[incv + 3] << 0) & 0xff));
			      incv += 4;
			      int size = ((int)((prxdata[incv + 0] << 24) & 0xff000000) |
			          (int)((prxdata[incv + 1] << 16) & 0xff0000) |
			          (int)((prxdata[incv + 2] << 8) & 0xff00) |
			          (int)((prxdata[incv + 3] << 0) & 0xff));
			      incv += 4;
			      int crc = ((int)((prxdata[incv + 0] << 24) & 0xff000000) |
			          (int)((prxdata[incv + 1] << 16) & 0xff0000) |
			          (int)((prxdata[incv + 2] << 8) & 0xff00) |
			          (int)((prxdata[incv + 3] << 0) & 0xff));
			      incv += 4;
			
			      logp("get userdata info size %d, type %d, crc 0x%x \n", size, type, crc);
			      
			    	int retsize = netRecvUData(utemplate, type, 0, size, crc);
			    	if ( retsize > 0 ){
			    		char path[80];            		
			    		sprintf(path, "%s.bin", sg_cmd_param[3] );
			    		exfFileOut( path, utemplate, size );
			    	}
			    	else{
			    		logp("PKT_CMD_GET_UERFACE_USERDATA recv fail\n");
			    	}
			      
					}
					else{
						logp("PKT_CMD_GET_UERFACE_USERDATA cmd fail\n");
					}
			  }
			
				free(utemplate);
      }
    }
	}
	else{
		logp("not supported.\n");
	}
}

/**************************************************************
*
*/
void cli_do_cam(void* _param)
{
	unsigned char txdata[1024];
	unsigned short cmd;
	int txdatalen;
	int pktsize;


	if ( sg_cmd_param[1][0] == 0 ){
		logp("not supported.\n");
		return;
	}
	
	if( !strcmp(sg_cmd_param[1], "read") ){
		if ( sg_cmd_param[2][0] != 0 ){
			netRecvCamData( sg_cmd_param[2] );
		}
		else{
			netRecvCamData( NULL );
		}
	}

	else if( !strcmp(sg_cmd_param[1], "led") ){
		commClearBuff();
		if ( sg_cmd_param[2][0] != 0 ){
			if( !strcmp(sg_cmd_param[2], "on") ){
				txdata[0] = 0;
				txdata[1] = 0;
				txdata[2] = 0;
				txdata[3] = 1;

				pktsize = pktMakeTxData( PKT_CMD_UERFACE_IRLED, txdata, 4, 0 );
			}
			else{
				txdata[0] = 0;
				txdata[1] = 0;
				txdata[2] = 0;
				txdata[3] = 0;
				pktsize = pktMakeTxData( PKT_CMD_UERFACE_IRLED, txdata, 4, 0 );
			}
		}
		else{			
			pktsize = pktMakeTxData( PKT_CMD_UERFACE_IRLED, NULL, 0, 0 );
		}
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( (unsigned short)cmd == (unsigned short)(PKT_CMD_UERFACE_IRLED|PKT_REQUEST_OK) ){
					logp("led ok\n");
				}
				else{
					logp("led fail\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}

	else if( !strcmp(sg_cmd_param[1], "init") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_SET_CAMERA_INIT, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( (unsigned short)cmd == (unsigned short)(PKT_CMD_SET_CAMERA_INIT|PKT_REQUEST_OK) ){
					logp("init ok\n");
				}
				else{
					logp("init fail\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}

	else if( !strcmp(sg_cmd_param[1], "close") ){
		commClearBuff();
		pktsize = pktMakeTxData( PKT_CMD_SET_CAMERA_CLOSE, NULL, 0, 0 );
		if ( commWrite( pktGetUartTxData(), pktsize ) > 0 ){
			if ( commRecvPktData( TIMEOUT_DEFAULT_RECVDATA ) > 0 ){
				cmd = pktGetUartRxCmd();
				if ( (unsigned short)cmd == (unsigned short)(PKT_CMD_SET_CAMERA_CLOSE|PKT_REQUEST_OK) ){
					logp("close ok\n");
				}
				else{
					logp("close fail\n");
				}
			}
			else{
				logd("recv fail\n");
			}
		}
	}

	else{
		logp("not supported.\n");
	}
}

/**************************************************************
*
*/
void cli_do_test(void* _param)
{
	unsigned char btdata[64];

}

/**************************************************************
*
*/
void cli_main( char* cmdstr )
{
	int argc, cmdidx;

	argc = cli_cmd_parser( cmdstr );
	cmdidx = cli_check_cmd(sg_cmd_param[0]);
	//logd(" cmd  objcount  %d\n", argc );
	if(cmdidx != -1){
		sg_cmd_list[cmdidx].func(NULL);
	}

	memset( sg_cmd_buf, 0, sizeof(sg_cmd_buf) );
	memset( sg_cmd_param, 0, sizeof(sg_cmd_param) );
}


