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

#ifndef DOWNLOAD_OPTS_H_
#define DOWNLOAD_OPTS_H_

#include "youtube-dl.h"
#include "coro.h"

struct myprogress {
  int stream;
  struct timespec last_run_time;
  struct {
	  bool init;
	  coro_context parser_coro;
	  struct coro_stack parser_stack;
	  uint8_t *mm_parser_buffer;
	  size_t bytes_avail;
	  size_t max_size;
  };
};

struct download_parameters{
    struct download_request requests[NUMOFSTREAMS];
	int run;
	struct myprogress * prog;
	struct timeval play_out_buffer_len;
};

int downloadfiles(videourl url [] );
size_t getfilesize(char* url);

#endif /* DOWNLOAD_OPTS_H_ */
