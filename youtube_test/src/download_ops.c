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

#include <sys/select.h>
#include <sys/param.h>
#include "download_ops.h"
#include "helper.h"
#include "metrics.h"
#include "mm_parser.h"
#include "youtube-dl.h"
#include "attributes.h"
#include "timespec_convertor.h"

extern metrics metric;
extern struct program_arguments program_arguments;

#define STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES         6000
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     1

static size_t write_data(void *ptr, size_t size, void *userdata) {
	struct myprogress *p = (struct myprogress *)userdata;

	memcpy(p->mm_parser_buffer, ptr, size);
	p->bytes_avail = size;
	/* Tell the coroutines there is new data */
	if(!p->init) {
		p->init = true;
        sem_init(&p->ffmpeg_awaits_mutex, 0, 0);
        sem_init(&p->data_arrived_mutex, 0, 0);
        pthread_create(&p->ffmpeg_thread, NULL, mm_parser, p);
	}
    sem_post(&p->data_arrived_mutex);
    sem_wait(&p->ffmpeg_awaits_mutex);
	return size;
}

static int xferinfo(struct myprogress *progress, struct output_data* output)
{
	if (!program_arguments.instantaneous_output)
	    return 0;

	double dlspeed = output->request_args->buffer.used_size / timespec_to_seconds(&output->download_time);
	/* if verbose is set then print the status  */

	struct timespec now_time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now_time);


	// don't report too often
	if(get_time_spec_diff(&now_time, &progress->last_run_time) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL)
	{
		long long now_time_us = gettimelong();

		printf("YOUTUBEINTERIM;%ld;", (long)metric.htime/1000000);
		printf("%ld;", (long)(now_time_us - metric.stime));
		printf("%"PRIu64";", metric.TSnow * 1000);
		if(metric.numofstreams > 1)
		{
			  if(progress->stream==STREAM_AUDIO)
					  printf("AUDIO;");
			  else if(progress->stream==STREAM_VIDEO)
					  printf("VIDEO;");
		}
		else
			  printf("ALL;");
		printf("%"PRIu64";", metric.TSlist[progress->stream] * 1000);
		printf("%ld;", output->request_args->buffer.used_size);
		printf("%ld;", output->request_args->buffer.used_size);
		printf("%.0f;", dlspeed);
		// bytes per sec
		printf("%.0f;", metric.totalbytes[progress->stream] / ((double)(now_time_us - metric.stime) / 1000000.0));
		printf("%d;", metric.numofstalls);
		printf("%.0f;", (metric.numofstalls > 0 ? (metric.totalstalltime/metric.numofstalls) : 0)); // av stall duration
		printf("%.0f;", metric.totalstalltime);
		printf("%s;", metric.http_ver[progress->stream]);
		printf("%s;\n", metric.quic_ver[progress->stream]);
		progress->last_run_time = now_time;
	}
/*	  else if (dlnow==dltotal)
	  	printf("They are equal: %.0f %.0f %.0f\n", dlnow, dltotal, myp->lastdlbytes); 
	else if( myp->lastdlbytes!=dlnow)
	  	printf("They are not equal: %.0f %.0f %.0f\n", dlnow, dltotal, myp->lastdlbytes); 
*/

  return 0;
}

void put_error_metrics(int stream_index)
{
	metric.totalbytes[stream_index] = -1;
	metric.downloadtime[stream_index] = -1;
	if (metric.connection_time_s[stream_index] == 0)
	{
		metric.connection_time_s[stream_index] = -1;
	}
}

static int process_output(struct myprogress *prog, struct output_data *download_result, int stream_index)
{
	double download_time_s = timespec_to_seconds(&download_result->download_time);
	double total_bytes = download_result->request_args->buffer.used_size;

	metric.totalbytes[stream_index] += total_bytes;
	metric.downloadtime[stream_index] += download_time_s;

  	/* Get connection time and CDN IP - Only for the first connect */
	if(metric.connection_time_s[stream_index] == 0)
	{
		metric.connection_time_s[stream_index] = timespec_to_seconds(&download_result->connection_time);
	  	strcpy(metric.cdnip[stream_index], download_result->used_ip);
		strcpy(metric.quic_ver[stream_index], program_arguments.quic_version);
		strcpy(metric.http_ver[stream_index], download_result->http_header);
	}
	xferinfo(prog, download_result);
	return 1;
}

static void free_ffmpeg_threads(struct myprogress * prog){
	for(int i = 0; i < metric.numofstreams; i++){
		prog[i].bytes_avail = 0;
		sem_post(&prog[i].data_arrived_mutex);
	}
}

static int cleanup_download_loop(struct myprogress *prog, int num, int errorcode, int last_http_code)
{
	free_ffmpeg_threads(prog);

	metric.etime = gettimelong();
	for(int j =0; j<num; j++)
	{
//		printf("metric printing %d\n", url[j].playing); fflush(stdout);
//		exit(1);
	  	if(errorcode==ITWORKED)
	  	{
	  	    /* Calculate download rate */
	  	    if(metric.downloadtime[j] != -1 && metric.totalbytes[j] != -1)
	  	    	metric.downloadrate[j] = metric.totalbytes[j] / metric.downloadtime[j];
		    else
			metric.downloadrate[j]=-1;

            if(metric.url[j].playing && metric.errorcode==0) {

                if(last_http_code==200)
                {
                    if(metric.Tmin<0)
                        checkstall(true);
                }
                else if(last_http_code==0) {
                    metric.errorcode = LSQUIC_ENGINE_ERROR;
                }
                else
                    metric.errorcode = last_http_code;
            }
	  	}
	}
	free(prog);
	if(metric.errorcode == 0)
		metric.errorcode = errorcode;
	return metric.errorcode;
}

size_t getfilesize(char* url)
{
	struct download_request request = {
			.write_argument = NULL,
			.on_write = NULL,
			.header_only = 1,
			.max_write_size = 0,
			.url_request.url_data = url,
			.url_request.url_len = strlen(url),
			.buffer.allocated_size = 0
	};
	int status = start_downloading(quic_client, &request, 1);

	if(status < 0 ) {
		destroy_client(quic_client);
		return -1;
	}
	struct output_data* output_data = get_download_result(quic_client);
	if (output_data->error_code || output_data->http_code != 200)
	{
		if (output_data->http_code >= 400)
			metric.errorcode = ACCESS_DENIED_URL_CODE;
		destroy_output_data_container(output_data);
		destroy_client(quic_client);
		return -1;
	}

	size_t content_len = output_data->data_size;
	destroy_output_data_container(output_data);
	return content_len;
}

void form_url(struct url* request_url, videourl * video_url)
{
    char range[50];
    strcpy(request_url->url_data, video_url->url);
    if(metric.playout_buffer_seconds>0)
    {
        sprintf(range, "&range=%ld-%ld",video_url->range0, video_url->range1);
        strcat(request_url->url_data,range);
    }
    request_url->url_len = strlen(request_url->url_data);
}

int initialize_progress_url(struct download_request *request, videourl *url, struct myprogress *prog,
        struct download_request* new_requests, int* new_requests_count_out)
{
	form_url(&request->url_request, url);
	prog->last_run_time.tv_sec = 0;
	prog->last_run_time.tv_nsec = 0;

	/* set options */
    // test
    if (program_arguments.instantaneous_output)
        puts(request->url_request.url_data);

    new_requests[*new_requests_count_out] = *request;
    *new_requests_count_out += 1;
	return 1;

}

void reset_buffer(struct download_buffer* buffer, size_t required_size)
{
	buffer->used_size = 0;
    buffer->buffer = NULL;
    buffer->allocated_size = 0;
}

void adjust_range(videourl* url_ref)
{
    url_ref->range0 = url_ref->range1;
    /*If range1 is greater than zero, this is not the first fetch*/
    if(url_ref->range1 > 0)
        ++url_ref->range0;
        /*First fetch, set playing to 1 for the stream*/
    else
        url_ref->playing = 1;
}

#define BUFFER_ADDDITIONAL_SPACE 1024

void load_streams(videourl url [], struct download_parameters* download_files_values,
        struct download_request* new_requests, int* new_requests_count_out)
{
	for(int i = 0; i < metric.numofstreams; i++)
	{
		adjust_range(&url[i]);
        /*the stream is not playing, move on*/
		if(!url[i].playing)
			continue;
		else
			++(download_files_values->run);
		/*Time to download more, check separately for each stream to decide how much to download. */
		download_files_values->play_out_buffer_len = get_curr_playoutbuf_len_forstream(i);
		if(download_files_values->play_out_buffer_len.tv_sec > metric.playout_buffer_seconds)
			continue;
		size_t range_addition;
		if((metric.playout_buffer_seconds - download_files_values->play_out_buffer_len.tv_sec) >= LEN_CHUNK_MINIMUM)
			range_addition = url[i].bitrate*LEN_CHUNK_MINIMUM;
		else
			range_addition = url[i].bitrate*LEN_CHUNK_FETCH;
		url[i].range1 += range_addition;
		download_files_values->prog[i].max_size = url[i].bitrate* MAX(LEN_CHUNK_MINIMUM, LEN_CHUNK_FETCH);
		reset_buffer(&download_files_values->requests[i].buffer, range_addition + BUFFER_ADDDITIONAL_SPACE);

#ifdef DEBUG
		printf("Getting next chunk for stream %d with range : %ld - %ld\n", i, url[i].range0, url[i].range1);
#endif
		initialize_progress_url(&download_files_values->requests[i], &url[i], &download_files_values->prog[i],
                                new_requests, new_requests_count_out);
	}
}

void check_max_test_time()
{
	if(metric.initialprebuftime >= 0) {
		long long now = gettimelong();
		long long playback_start_time = metric.stime + metric.initialprebuftime;
		if((now - playback_start_time) / 1000000 > program_arguments.max_test_time)
			/*the test has been running for too long
            terminate the session. */
			metric.errorcode = MAXTESTRUNTIME;
	}
}

int find_request_index(struct download_request* requests, struct download_request* request)
{
	for (int idx = 0; idx < metric.numofstreams; idx++)
	{
		// url was allocated once here
		if (request->url_request.url_data == requests[idx].url_request.url_data)
			return idx;
	}
	return -1;
}

int update_stream_progress(videourl *url, int idx, struct output_data* output_result, struct myprogress *prog)
{
	// test ends on downloaded size less then requested
	if (output_result->request_args->buffer.used_size < url->range1 - url->range0)
	{
		#ifdef DEBUG
			printf("Full HTTP transfer completed for stream %d  with status %d\n", idx, msg->data.result);fflush(stdout);
        #endif
		url->playing = 0;
	}
	int status = 0;
	if(url->playing)
		status = process_output(prog, output_result, idx);
	return status;
}

int check_errors(struct output_data* output, int stream_index)
{
	if (output->error_code)
	{
		if (output->error_code == QUIC_CLIENT_CODE_TIMEOUT)
			metric.errorcode = LSQUIC_TIMEOUT_ERROR;
		else
			metric.errorcode = LSQUIC_ENGINE_ERROR;
		put_error_metrics(stream_index);
		if (program_arguments.is_connection_level_debug || program_arguments.instantaneous_output)
		{
			printf("error:%s\n", output->error_str);
		}
		return -1;
	}
}

int process_outputs(videourl *urls, struct download_parameters *download_params, int* last_http_code_out)
{
	struct download_request* requests = download_params->requests;

	struct output_data* output = get_download_result(quic_client);
	if (output == NULL)
		metric.errorcode = LSQUIC_ENGINE_ERROR;
	while ((output != NULL))
	{
		*last_http_code_out = output->http_code;
		/* Find out which handle this message is about */
		int idx = find_request_index(requests, output->request_args);
		if (idx < 0)
		{
			metric.errorcode = LSQUIC_ENGINE_ERROR;
		}
		int status = check_errors(output, idx);
		if (status < 0)
			return status;
		status = update_stream_progress(&urls[idx], idx, output, &download_params->prog[idx]);
		if (status < 0)
			return status;
		destroy_output_data_container(output);
		output = get_download_result(quic_client);
	}
	check_max_test_time();
	if(metric.errorcode)
	{
		download_params->run = 0;
		return -1;
	}
	/*checkstall should not be called here, more than one TS might be received during one session, it is important
	 * that we check the status of the playout buffer as soon as a new frame(TS) is received checkstall(false);*/

	return 0;
}

void init_zero_buffer(struct download_buffer* buffer_shell)
{
    buffer_shell->buffer = NULL;
    buffer_shell->used_size = 0;
    buffer_shell->allocated_size = 0;
}

#define URL_LEN 1500

void init_requests(struct download_request *requests, struct myprogress *progress_array)
{
    for(int i = 0; i < NUMOFSTREAMS; i++) {
        init_zero_buffer(&requests[i].buffer);
        requests[i].on_write = write_data;
        requests[i].write_argument = &progress_array[i];
        requests[i].max_write_size = CURL_MAX_WRITE_SIZE;
		requests[i].header_only = 0;
        requests[i].url_request.url_data = malloc(URL_LEN);
    }

}

void init_progs(struct myprogress* progress_array)
{
    for(int i = 0; i < NUMOFSTREAMS; i++) {
        progress_array[i].stream = i;
        progress_array[i].init = false;
    }
}

void destroy_requests(struct download_request* requests)
{
	for(int i = 0; i < NUMOFSTREAMS; i++) {
		free(requests->buffer.buffer);
		requests->buffer.buffer = NULL;
		free(requests->url_request.url_data);
        requests->url_request.url_data = NULL;
	}
}

int downloadfiles(videourl url [] )
{
	struct download_parameters download_files_values = {
		.run = metric.numofstreams,
        .prog = malloc(sizeof(struct myprogress [metric.numofstreams]))
	};
	metric.errorcode = 0;
    init_requests(download_files_values.requests, download_files_values.prog);
    init_progs(download_files_values.prog);
	metric.stime = gettimelong();

    struct download_request new_requests[metric.numofstreams];

	int last_http_code = 0;
	while(download_files_values.run)
	{
        int new_requests_count = 0;
		download_files_values.run = 0;
		/*Previous transfers have finished, figure out if we have enough space in the buffer to download 
		  more or should we wait while the buffer empties */
		/*Get the length of the current buffer*/
		download_files_values.play_out_buffer_len = get_curr_playoutbuf_len();
		/*Adjust timetowait based on the length of the playout buffer, we don't care about usecs*/
		if(download_files_values.play_out_buffer_len.tv_sec>metric.playout_buffer_seconds)
		{
			download_files_values.play_out_buffer_len.tv_sec =
					download_files_values.play_out_buffer_len.tv_sec-metric.playout_buffer_seconds;
#ifdef DEBUG
			printf("Time to wait: %d seconds. Sleeping at %ld....", playoutbuffer_len.tv_sec, gettimeshort());
#endif
			select(0, NULL, NULL, NULL, &download_files_values.play_out_buffer_len);
#ifdef DEBUG
			printf("Awake %ld\n", gettimeshort());
#endif
		}

		load_streams(url, &download_files_values, new_requests, &new_requests_count);

		if(download_files_values.run == 0)
			break; 
		/* we start some action by calling perform right away */

        if (new_requests_count > 0)
            if (start_downloading(quic_client, new_requests, new_requests_count) < 0) {
                metric.errorcode = LSQUIC_ENGINE_ERROR;
                break;
            }

		if (process_outputs(url, &download_files_values, &last_http_code) < 0) {
			break;
		}
	}
	destroy_requests(download_files_values.requests);
	return cleanup_download_loop(download_files_values.prog, metric.numofstreams, ITWORKED, 200);
}
