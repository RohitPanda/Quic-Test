/*
 *
 *      Year   : 2013-2017
 *      Author : Saba Ahsan
 *               Cristian Morales Vega
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#ifndef YOUTUBE_DL_H_
#define YOUTUBE_DL_H_
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <pthread.h>
#include <curl/curl.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "arguments_parser.h"

//#define DEBUG 1
#define ITWORKED 600
#define WERSCREWED 666
#define BADFORMATS 601 //we didn't find the right format for download.
#define MAXDOWNLOADLENGTH 603 /*three minutes of video has been downloaded*/
#define MAXTESTRUNTIME 604 /*test has been downloading video for the maximum time */
#define SIGNALHIT 605 /* program received ctrl+c*/
#define CODETROUBLE 606 /*HTTP code couldn't be retrieved*/
#define ERROR_STALL 607 /* There was a stall */
#define TOO_FAST 608 /* The test finsihed too fast */
#define PARSERROR 609 /*There was an error with the initial HTTP response*/
#define CURLERROR 610
#define CURLERROR_GETINFO 611
#define FIRSTRESPONSERROR 620
#define ACCESS_DENIED 621

#define LEN_PLAYOUT_BUFFER 40 /*Length of the playout buffer in seconds*/
#define LEN_CHUNK_FETCH 1 /*Length to be requested to refill buffer*/



#define NUMOFSTREAMS 2
#define STREAM_VIDEO 0
#define STREAM_AUDIO 1



#define NUMOFCODES 9
#define MIN_PREBUFFER 2000 /* in millisecond*/
#define MIN_STALLBUFFER 1000

/*the number of filetype enum indicates priority, the lower the value, the higher the priority*/
typedef enum {MP4=3, WEBM=4, FLV=5, TGPP=6, MP4_A=1, WEBM_A=2, NOTSUPPORTED=7} filetype;

#define URLTYPELEN 100
#define URLSIZELEN 12
#define URLSIGLEN 100
#define CDNURLLEN 1500
#define URLLISTSIZE 24

typedef struct
{
	int itag;
	char url[CDNURLLEN];
	char type[URLTYPELEN];
	int bitrate;
/*for the range parameter in the YouTube url*/
	long range0; /*first byte to be fetched*/
	long range1; /*last byte to be fetched*/
	int playing; 
} videourl;

#define CHARPARSHORT 12
#define CHARPARLENGTH 24
#define CHARSTRLENGTH 256
#define FORMATLISTLEN 100


typedef struct
{
	videourl url[NUMOFSTREAMS];
	videourl adap_videourl[URLLISTSIZE];
	videourl adap_audiourl[URLLISTSIZE];
	videourl no_adap_url[URLLISTSIZE];
	filetype ft;
	int numofstreams;
	char link[MAXURLLENGTH];
	long long htime; /*unix timestamp when test began*/
	long long stime; /*unix timestamp in microseconds, when media download began*/
	long long etime;/*unix timestamp in microseconds, when test ended*/
	long long startup; /*time in microseconds, stime-htime for first media download only*/ 
	//char url[CDNURLLEN];
	int numofstalls;
	char cdnip[NUMOFSTREAMS][CHARSTRLENGTH];
	double totalstalltime; //in microseconds
	double initialprebuftime; //in microseconds
	double downloadtime[NUMOFSTREAMS]; //time take for video to download
	double totalbytes[NUMOFSTREAMS];
	uint64_t TSnow; //TS now (in milliseconds)
	uint64_t TSlist[NUMOFSTREAMS]; //TS now (in milliseconds)
	uint64_t TS0; //TS when prebuffering started (in milliseconds). Would be 0 at start of movie, but would be start of stall time otherwise when stall occurs.
	long long Tmin; // microseconds when prebuffering done or in other words playout began.
	long long Tmin0; //microseconds when prebuffering started, Tmin0-Tmin should give you the time it took to prebuffer.
	long long T0; /*Unix timestamp when first packet arrived in microseconds*/
	int dur_spec; /*duration in video specs*/
	double downloadrate[NUMOFSTREAMS];
	int errorcode;
	double tcp_connection_time_us[NUMOFSTREAMS]; // curl time_connect - time just for tcp connect
	double full_ssl_connection_time_us[NUMOFSTREAMS]; // curl time_appconnect - The time, it took from the start until
	// the SSL/SSH/etc connect/handshake to the remote  host was completed
	double tcp_first_connection_time_us;
	double first_full_connection_time_us; // curl time_appconnect - The time, in seconds, it took from the start until
	// the SSL/SSH/etc connect/handshake to the remote  host was completed
	bool fail_on_stall;
	int playout_buffer_seconds; 
} metrics;

#endif /* YOUTUBE_DL_H_ */
