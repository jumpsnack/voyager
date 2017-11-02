/*
  pkt.c - linux sample code for voyager
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

static unsigned char* sg_tx_data;
static int sg_tx_data_len;

static unsigned short sg_rx_cmd;
static unsigned short sg_16_dummy;

static int sg_rx_datalen;
static unsigned char* sg_rx_data;


#define PKT_WAIT_TIMEOUT	(50)
/* send uart data-packet
 *
 */
unsigned char pktGetCheckSum(unsigned char *buff, unsigned int cnt)
{
	unsigned int pos;
	unsigned char cs = 0;

	for(pos = 1 ; pos < cnt ; pos++)
		cs ^= buff[pos];
	return cs;
}

int pktGetCrc32( char* buff, int cnt )
{
	int crc32 = 0;
	int i;

	for(i=0;i<cnt;i++){
		crc32 += (int)buff[i];
	}

	return crc32;
}

unsigned short pktGetUartRxCmd(void)
{
	return sg_rx_cmd;
}

unsigned char* pktGetUartRxData(void)
{
	return sg_rx_data;
}

int pktGetUartRxDatalen(void)
{
	return sg_rx_datalen;
}

unsigned char* pktGetUartTxData(void)
{
	return sg_tx_data;
}

int pktGetUartTxDatalen(void)
{
	return sg_tx_data_len;
}

int pktMakeTxData(unsigned short cmd, unsigned char* data, int dlen, int pwcode)
{
	int inc = 0;
	int i;

	memset(sg_tx_data, 0, PKT_MAX_SIZE);
	
	sg_tx_data[inc] = PKT_SOP;

	sg_tx_data[++inc] = 0; // reserve space. (p-len)
	sg_tx_data[++inc] = 0;

	sg_tx_data[++inc] = (unsigned char)((cmd>>8) & 0xff);
	sg_tx_data[++inc] = (unsigned char)((cmd>>0) & 0xff);

	sg_tx_data[++inc] = (unsigned char)((dlen>>24) & 0xff);
	sg_tx_data[++inc] = (unsigned char)((dlen>>16) & 0xff);
	sg_tx_data[++inc] = (unsigned char)((dlen>>8) & 0xff);
	sg_tx_data[++inc] = (unsigned char)((dlen>>0) & 0xff);

	for( i=0; i<dlen; i++){
		sg_tx_data[++inc] = data[i];
	}

//	sg_tx_data[1] = (unsigned char)( ((inc+1)>>8) & 0xff);
//	sg_tx_data[2] = (unsigned char)( ((inc+1)>>0) & 0xff);
	sg_tx_data[1] = (unsigned char)( ((inc+7)>>8) & 0xff);
	sg_tx_data[2] = (unsigned char)( ((inc+7)>>0) & 0xff);

	sg_tx_data[++inc] = 0;
	for(i=1;i<inc;i++){
		sg_tx_data[inc] ^= sg_tx_data[i];
	}

	// sec field (4bytes)
	sg_tx_data[++inc] = (unsigned char)((pwcode>>24) & 0xff);
	sg_tx_data[++inc] = (unsigned char)((pwcode>>16) & 0xff);
	sg_tx_data[++inc] = (unsigned char)((pwcode>>8) & 0xff);
	sg_tx_data[++inc] = (unsigned char)((pwcode>>0) & 0xff);

	sg_tx_data[++inc] = PKT_EOP;
	sg_tx_data_len = inc+1;
	return sg_tx_data_len;
}

short pktParseRxPktLen( unsigned char* rxpkt )
{
	return (short)(( rxpkt[1]<<8 ) & 0xff00) | (( rxpkt[2]<<0 ) & 0xff);
}

int pktParseRxData( unsigned char* rxpkt )
{
	unsigned char crc;
	int i;
	short plen;
	int incv=1;

	crc = 0;

	memset ( sg_rx_data, 0, PKT_MAX_SIZE );
	sg_rx_datalen = 0;

	if ( rxpkt == NULL ){
		return -1;
	}

	plen  = (short)((( rxpkt[incv+0]<<8 ) & 0xff00) | (( rxpkt[incv+1]<<0 ) & 0xff));
	if( plen < 0 || plen > (short)PKT_MAX_SIZE ){
		return -1;
	}
	incv+=2;

	//cmd
	sg_rx_cmd = (unsigned short)((( rxpkt[incv+0]<<8 ) & 0xff00) | (( rxpkt[incv+1]<<0 ) & 0xff));
	incv+=2;

	//datalen
	sg_rx_datalen = (int)( (( rxpkt[incv+0]<<24 ) & 0xff000000)
			| (( rxpkt[incv+1]<<16 ) & 0xff0000)
			| (( rxpkt[incv+2]<<8 ) & 0xff00)
			| (( rxpkt[incv+3]<<0 ) & 0xff));
	incv+=4;
	//data
	if( sg_rx_datalen < 0 || sg_rx_datalen > (int)PKT_MAX_SIZE )
		return -1;
	
	memcpy( sg_rx_data, &rxpkt[incv], sg_rx_datalen ) ;
	incv+=sg_rx_datalen;

	for( i=1; i<incv; i++ ){
		crc ^= rxpkt[i];
	}

	if ( rxpkt[incv] != crc ){
		return -2;
	}

	if ( (unsigned char)rxpkt[incv+1] != (unsigned char)PKT_EOP ){
		return -3;
	}

	return 0;
}

int pktClose()
{
	free( sg_rx_data );
	free( sg_tx_data );
	return 0;
}

int pktInit()
{
	sg_rx_data = malloc( PKT_MAX_SIZE );
	sg_tx_data = malloc( PKT_MAX_SIZE );
	return 0;
}


