/*
  app.h - linux sample code for voyager
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
#ifndef __APP_H__
#define __APP_H__

#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <malloc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>  //clock_gettime  compile ver.
//#include <linux/time.h>  //clock_gettime  edit ver.
#include <dirent.h>
#include <sys/utsname.h>
#include <sys/mman.h>


// only ansi
#define iMalloc malloc
#define iFree free

#define logp(fmt,args...) printf(fmt,##args)
#define logd(fmt,args...) printf("[%s:%d] : " fmt, __FILE__, __LINE__, ##args)

#define TIME_CHECK_ST()	do{  \
		unsigned long __sec, __nsec, __ov=0; \
		struct timespec  __t0, __t1; \
		clock_gettime(CLOCK_MONOTONIC, &__t0);

#define TIME_CHECK_EN()	 \
	clock_gettime(CLOCK_MONOTONIC, &__t1); \
	__t1.tv_nsec > __t0.tv_nsec ? \
	__nsec = (__t1.tv_nsec - __t0.tv_nsec) : ( (__nsec = (__t1.tv_nsec+1000000000) - __t0.tv_nsec), __ov=1); \
	__sec = __t1.tv_sec - __t0.tv_sec - __ov; \
	printf("[%ld.%09d]%s]%s L:%d# time %ld.%09ld(sec) \n", __t1.tv_sec, __t1.tv_nsec ,  __FILE__, __FUNCTION__, __LINE__, \
	__sec, __nsec); \
	}while(0);

#define COMM_MODE_STATUS_NORMAL	(0)
#define COMM_MODE_STATUS_NORMAL_ON	(1)
#define COMM_MODE_STATUS_QUEUING	(2)
//#define COMM_MODE_STATUS_QUEUING_ON	(3)

#define TIMEOUT_DEFAULT_RECVDATA	(500)
#define TIMEOUT_STREAMDATA	(10000) // 10sec

#define UART_DEFAULT_BAUDRATE	(115200)

#define UART_MAX_IMGBUFF_RX_BUFF	(1024*1024*3)
#define UART_MAX_RX_BUFF	(524288)
#define UART_MAX_TX_BUFF	(2048)

#define PKT_MAX_SIZE	(2048) //(524288)//



// packet mode
#define PKT_SOP (0xAA)
#define PKT_EOP (0x55)

#define PKT_SOP_LEN (1)
#define PKT_EOP_LEN (1)
#define PKT_PLEN_LEN	(2)
#define PKT_CMD_LEN	(2)
#define PKT_CRC_LEN	(1)
#define PKT_DLEN_LEN	(4)

#define PKT_CMD_TEST 0x0100
#define PKT_CMD_REBOOT 0x0101
#define PKT_SEC_TEST 0x0110
#define PKT_CMD_FACTRESET 0x0150

#define PKT_CMD_RESEND 0x1500

#define PKT_REQUEST_OK 0x4000
#define PKT_REQUEST_ERR 0x8000

#define PKT_CMD_LOGIN_USER 0x3000
#define PKT_CMD_LOGOUT_USER 0x3001
#define PKT_CMD_SET_USER_PW 0x3011
#define PKT_CMD_SET_BAUD 0x3012
#define PKT_CMD_GET_BAUD 0x3013
#define PKT_CMD_ALIVE 0x3014
#define PKT_CMD_SET_CAMERA_INIT 0x3019
#define PKT_CMD_SET_CAMERA_CLOSE 0x301A
#define PKT_CMD_UPDATE_APP	0x3020
#define PKT_CMD_SET_SYSTIME	0x3021
#define PKT_CMD_GET_APPVER	0x3022
#define PKT_CMD_GET_UERFACE_VER	0x3023
#define PKT_CMD_SET_PKT_FRAME_LENGTH	0x3024

#define PKT_CMD_UPDATE_SECBIN	0x3031
#define PKT_CMD_GET_SECBIN_INFO	0x3032

#define PKT_CMD_SEND_FILE_INFO 0x1001
#define PKT_CMD_RECV_CAM_INFO	0x1005

#define PKT_CMD_TRANSFER_READY 0x1007
#define PKT_CMD_TRANSFER_DATA 0x1008
#define PKT_CMD_TRANSFER_COMPLETE 0x1009

#define PKT_CMD_UERFACE_USERADD 0x2001    // reg
#define PKT_CMD_UERFACE_USERCHECK 0x2002  // rec
#define PKT_CMD_UERFACE_FACECHECK 0x2003  // fd
#define PKT_CMD_UERFACE_USERLIST 0x2004
#define PKT_CMD_UERFACE_USERDEL 0x2005
#define PKT_CMD_UERFACE_INIT	0x2006
#define PKT_CMD_UERFACE_STATUS	0x2007
#define PKT_CMD_UERFACE_RESET	0x2008

#define PKT_CMD_UERFACE_USERADD_INIT 0x2009    // reg
#define PKT_CMD_UERFACE_USERCHECK_INIT 0x200a  // rec
#define PKT_CMD_UERFACE_FACECHECK_INIT 0x200b  // fd
#define PKT_CMD_UERFACE_IRLED 0x200c  //
#define PKT_CMD_UERFACE_USERADD_END 0x200d    // reg
#define PKT_CMD_UERFACE_USERCHECK_END 0x200e  // rec
#define PKT_CMD_UERFACE_FACECHECK_END 0x200f  // fd
#define PKT_CMD_UERFACE_USERADD_PREPROCESS 0x2010
#define PKT_CMD_UERFACE_USERADD_SAVE 0x2014    // reg save
#define PKT_CMD_UERFACE_USERADD_ROLLBACK 0x2015    // reg rollback

#define PKT_CMD_UERFACE_SET_SECLEVEL 0x2020    //
#define PKT_CMD_UERFACE_GET_SECLEVEL 0x2021    //

#define PKT_CMD_UERFACE_CAM_STATUS 0x2022

// added 2017.6.22
#define PKT_CMD_UERFACE_FREERUN_TIMEOUT 0x2030  // infinite time out.
#define PKT_CMD_UERFACE_FREERUN_USERADD 0x2031    // reg-infinite
#define PKT_CMD_UERFACE_FREERUN_USERCHECK 0x2032  // rec-infinite

// added 2017.6.28
#define PKT_CMD_UERFACE_UPDATE_USERNAME	0x2016
#define PKT_CMD_GET_UERFACE_USERDATA	0x2040
#define PKT_CMD_ENROLL_UERFACE_USERDATA	0x2042

#define UF_USERDATA_INFO_SIZE	(32)
typedef struct _uf_userdata_info{
	char name[20];
	int type;
	int size;
	int crc;
}UfUserDataInfo;  // 32bytes

#define PKT_CMD_ECHO_SEC_KEY_PLAINDATA	0x3040
#define PKT_CMD_SET_SEC_KEY		0x3041


#define APP_ERR_CODE_NOT_YET	(-1000)
#define APP_ERR_CODE_NO_COMMAND	(-1001)



#endif