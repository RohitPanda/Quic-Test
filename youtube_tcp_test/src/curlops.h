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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CURLOPS_H_
#define CURLOPS_H_

#include <semaphore.h>
#include "youtube-dl.h"

struct myprogress {
  int stream;
  long lastruntime;
  double lastdlbytes;
  CURL *curl;
  struct {
	  bool init;
	  pthread_t ffmpeg_thread;
	  sem_t data_arrived_mutex;
	  sem_t ffmpeg_awaits_mutex;
	  uint8_t *curl_buffer;
	  size_t bytes_avail;
  };
};

struct download_handles{
	CURL *http_handle[NUMOFSTREAMS];
	CURLM *multi_handle;
};

struct download_parameters{
	struct download_handles handles;
	int still_running; /* keep number of running handles */
	int run;
	struct myprogress * prog;
	struct timeval playoutbuffer_len;
};

int downloadfiles(videourl url [] );
long getfilesize(const char url[]);
int set_ip_version(CURL *curl, enum IPv ip_version);

#endif /* CURLOPS_H_ */
