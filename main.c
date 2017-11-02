/*
  main.c - linux sample code for voyager
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
#include <sys/time.h>
#include "app.h"

static char sg_admin_curr_pw[64];

void exfFileOut( char* path, char* data, int len )
{
	FILE *fp;

	fp = fopen ( path, "wb" );
	if( !fp )
	{
		printf(" -- not foundded path -- \n");
		return;
	}

	fwrite( data, len, 1, fp );
	fflush( fp );

	fclose (fp);

	sync();
	chmod(path, 0666);
}

int exfFileSize( char* path )
{
	int size;
	// ansi
	FILE *fp;

	fp = fopen ( path, "rb" );
	if( !fp )
	{
		printf(" -- not foundded path -- \n");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fclose ( fp );

	return size;
}

int exfFileIn( char* path, char* data, int len )
{
	int readCnt;
	// ansi
	FILE *fp;

	fp = fopen ( path, "rb" );
	if( !fp )
	{
		printf(" -- not foundded path -- \n");
		return -1;
	}

	readCnt = fread( data, len, 1, fp );
	fclose( fp );
	return readCnt;
}



unsigned long long getTimems()
{
	struct timeval val;
	unsigned long long ms;

	gettimeofday(&val, NULL);
	ms = val.tv_sec * 1000;
	ms += val.tv_usec / 1000;
	return ms;
}

int main( int argc, char* argv[] )
{
	int ret;
	int trycount;
	char cmd[256];	
	int cmdlen;

	unsigned short pktcmd;

	if( argc < 3 ){
		logp("\nUsage Remote Voyager CLI\n");
		logp("execute : \n");
		logp(" > remotevoy [dev] [baudrate]  -  '[]' remove \n");
		logp("CMD : \n");
		logp(" > exit - EXIT\n");
		return -1;
	}

	pktInit();

	if ( commOpen( argv[1], atoi(argv[2]) ) < 0 ){
		logp("comm open failed\n");
		return -1;
	}

	do{
		memset(cmd, 0, 256);
		logp("cmd# ");
		fgets(cmd, 256, stdin);
		fflush(stdin);
		fflush(stdout);
		cmdlen = strlen(cmd)-1;
		cmd[cmdlen] = '\0';
		//logp("input cmd : %s\n", cmd);

		if ( !strcmp( cmd, "exit" ) ){
			break;
		}

		cli_main( cmd );
	}while(1);

	commClose();
	pktClose();
	return 0;
}
