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

#include "curlops.h"
#include "helper.h"
#include "metrics.h"
#include "mm_parser.h"
#include "youtube-dl.h"
#include "attributes.h"
#include <sys/select.h>

//#define SAVEFILE

extern metrics metric;
extern struct program_arguments program_arguments;

#define STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES         6000
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     1

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
	struct myprogress *p = (struct myprogress *)userdata;
	size_t len =size*nmemb;

	p->curl_buffer = (uint8_t*)ptr;
	p->bytes_avail = len;
	/* Tell the coroutines there is new data */
	if(!p->init) {
		p->init = true;
		sem_init(&p->ffmpeg_awaits_mutex, 0, 0);
		sem_init(&p->data_arrived_mutex, 0, 0);
		pthread_create(&p->ffmpeg_thread, NULL, mm_parser, p);
	}
	sem_post(&p->data_arrived_mutex);
	sem_wait(&p->ffmpeg_awaits_mutex);
	return len;
}


static void my_curl_easy_returnhandler(CURLcode eret, int i)
{
	if(eret != CURLE_OK)
		fprintf(stderr, "curl_easy_setopt() failed for stream %d with %d: %s\n",i, eret, curl_easy_strerror(eret));
}

static int xferinfo(void *p,
                    curl_off_t dltotal, curl_off_t dlnow,
                    curl_off_t UNUSED(ultotal), curl_off_t UNUSED(ulnow))
{
  struct myprogress *myp = (struct myprogress *)p;
  CURL *curl = myp->curl;
  double totaltime, starttime, curtime, dlspeed;

  curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &starttime);
  if(starttime == 0) {
          return 0;
  }
  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &totaltime);
  curtime = totaltime - starttime;
  curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &dlspeed);
  /* if verbose is set then print the status  */
  if(program_arguments.instantaneous_output)
  {
          if(((curtime - myp->lastruntime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL) || (dlnow==dltotal && myp->lastdlbytes!=dlnow && dlnow!=0))
          {
		  long long timenow = gettimelong(); 
                  printf("YOUTUBEINTERIM;%ld;", (long)metric.htime/1000000);
                  printf("%ld;", (long)(timenow- metric.stime));
                  printf("%"PRIu64";", metric.TSnow * 1000);
                  if(metric.numofstreams > 1)
                  {
                          if(myp->stream==STREAM_AUDIO)
                                  printf("AUDIO;");
                          else if(myp->stream==STREAM_VIDEO)
                                  printf("VIDEO;");
                  }
                  else
                          printf("ALL;");
                  printf("%"PRIu64";", metric.TSlist[myp->stream] * 1000);
                  printf("%ld;", (long)dlnow);
                  printf("%ld;", (long)dltotal);
		  printf("%.0f;", dlspeed);
                  printf("%.0f;", (dlnow-myp->lastdlbytes)/(curtime-myp->lastruntime));
		  printf("%.0f;", metric.totalbytes[myp->stream] / ((double)(timenow - metric.stime) / 1000000));
                  printf("%d;", metric.numofstalls);
                  printf("%.0f;", (metric.numofstalls>0 ? (metric.totalstalltime/metric.numofstalls) : 0)); // av stall duration
                  printf("%.0f\n", metric.totalstalltime);
                  myp->lastdlbytes = dlnow;
                  myp->lastruntime = curtime;
          }
/*	  else if (dlnow==dltotal)
	  	printf("They are equal: %.0f %.0f %.0f\n", dlnow, dltotal, myp->lastdlbytes); 
	else if( myp->lastdlbytes!=dlnow)
	  	printf("They are not equal: %.0f %.0f %.0f\n", dlnow, dltotal, myp->lastdlbytes); 
*/  }

  return 0;
}

/* for libcurl older than 7.32.0 (CURLOPT_PROGRESSFUNCTION) */
static int older_progress(void *p,
                          double dltotal, double dlnow,
                          double ultotal, double ulnow)
{
  return xferinfo(p,
                  (curl_off_t)dltotal,
                  (curl_off_t)dlnow,
                  (curl_off_t)ultotal,
                  (curl_off_t)ulnow);
}


static int update_curl_progress(struct myprogress * prog, CURL *http_handle[], int j)
{
	double starttransfer, downloadtime, totalbytes;
  	double lookup_time;
	/*Get total bytes*/
	if( curl_easy_getinfo(http_handle[j], CURLINFO_SIZE_DOWNLOAD, &totalbytes)!= CURLE_OK)
		metric.totalbytes[j]=-1;
	if(metric.totalbytes[j]!=-1)
		metric.totalbytes[j] += totalbytes;
	/* Get download time */
        if( curl_easy_getinfo(http_handle[j], CURLINFO_TOTAL_TIME, &downloadtime)!= CURLE_OK)
                metric.downloadtime[j]=-1;
        if( curl_easy_getinfo(http_handle[j], CURLINFO_STARTTRANSFER_TIME, &starttransfer)!= CURLE_OK)
                metric.downloadtime[j]=-1;

        if(metric.downloadtime[j] != -1)
		{
			downloadtime-= starttransfer;
			metric.downloadtime[j] += downloadtime;
		}
  	/* Get connection time and CDN IP - Only for the first connect */
	if(metric.tcp_connection_time_us[j]==0)
	{
		if( curl_easy_getinfo(http_handle[j], CURLINFO_CONNECT_TIME, &metric.tcp_connection_time_us[j])!= CURLE_OK)
		    metric.tcp_connection_time_us[j] =- 1;
		else {
			if( curl_easy_getinfo(http_handle[j], CURLINFO_NAMELOOKUP_TIME, &lookup_time)!= CURLE_OK) {
		  	    	metric.tcp_connection_time_us[j] =- 1;
		  	} else {
		  		metric.tcp_connection_time_us[j] -= lookup_time;
		  	}
		}
	  	char * remoteip;
	  	curl_easy_getinfo (http_handle[j], CURLINFO_PRIMARY_IP, &remoteip);
	  	strcpy(metric.cdnip[j], remoteip);
	}
	/* Get connection time and CDN IP - Only for the first connect */
	if(metric.full_ssl_connection_time_us[j]==0)
	{
		if(curl_easy_getinfo(http_handle[j], CURLINFO_APPCONNECT_TIME, &metric.full_ssl_connection_time_us[j])!= CURLE_OK)
			metric.full_ssl_connection_time_us[j] =- 1;
	}
	return 1; 

}

static void free_ffmpeg_threads(struct myprogress * prog){
	for(int i = 0; i < metric.numofstreams; i++){
		prog[i].curl_buffer = NULL;
		prog[i].bytes_avail = 0;
		sem_post(&prog[i].data_arrived_mutex);
	}
}

static int my_curl_cleanup(struct myprogress * prog, CURLM * multi_handle, CURL *http_handle[], int num, int errorcode,
		videourl url [])
{
	free_ffmpeg_threads(prog);

	metric.etime = gettimelong();
	long http_code;
	for(int j =0; j<num; j++)
	{
//		printf("metric printing %d\n", url[j].playing); fflush(stdout); 
//		exit(1); 
		if(metric.url[j].playing)
		    update_curl_progress(prog, http_handle, j);
	  	if(errorcode==ITWORKED)
	  	{
	  	    /* Calculate download rate */
	  	    if(metric.downloadtime[j] != -1 && metric.totalbytes[j] != -1)
	  	    	metric.downloadrate[j] = metric.totalbytes[j] / metric.downloadtime[j];
		    else
			metric.downloadrate[j]=-1; 

            if(metric.url[j].playing && metric.errorcode==0){
	  	    if( curl_easy_getinfo (http_handle[j], CURLINFO_RESPONSE_CODE, &http_code) == CURLE_OK) {
	  	    	if(http_code == 200) {
	  	    		if(metric.Tmin < 0)
	  	    			checkstall(true);
	  	    	}
	  	    	else if(http_code == 0) {
	  	    		metric.errorcode = CURLERROR;
	  	    	}
	  	    	else
	  	    		metric.errorcode = http_code;
            }
	  	    else
	  	    	metric.errorcode = CODETROUBLE;
            }
	  	}
	  	curl_easy_cleanup(http_handle[j]);
	}
	curl_multi_cleanup(multi_handle);
	free(prog);
//	if(metric.errorcode==0 || metric.errorcode==MAXTESTRUNTIME)
	if(metric.errorcode == 0)
		metric.errorcode = errorcode;
	return metric.errorcode;
}

int set_ip_version(CURL *curl, enum IPv ip_version) {
	int ret = 0;

	CURLcode res;
	switch(ip_version) {
	case IPv4:
		res = curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
		if(res != CURLE_OK) {
			ret = -1;
		}
		break;
	case IPv6:
		res = curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
		if(res != CURLE_OK) {
			ret = -1;
		}
		break;
	case IPvSYSTEM:
		break;
	default:
		ret = -2;
	}

	return ret;
}

long getfilesize(const char* url)
{
	long ret;

	CURL *curl_handle = curl_easy_init();
	if(curl_handle == NULL) {
		curl_easy_cleanup(curl_handle);
		return -1;
	}

	CURLcode error = 0;
	/* if redirected, tell libcurl to follow redirection */
	error |= curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
	/* timeout if no response is received in 1 minute */
	error |= curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 60);
	/* just get the header for the file size */
	error |= curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);
	error |= curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	error |= curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);

	if(error) {
		curl_easy_cleanup(curl_handle);
		return -2;
	}

	/* request to download the file */
	CURLcode c = curl_easy_perform(curl_handle);
	if(c != CURLE_OK) {
		fprintf(stderr, "curl failed with %d: %s %s\n",c, curl_easy_strerror(c), url);
		curl_easy_cleanup(curl_handle);
		return -3;
	}

	long http_code;
	error = curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
	if(http_code != 200 || error != CURLE_OK) {
		curl_easy_cleanup(curl_handle);
		if (http_code >= 400)
			metric.errorcode = ACCESS_DENIED;
		return -4;
	}

	double content_length;
	error = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &content_length);
	if(error != CURLE_OK) {
		curl_easy_cleanup(curl_handle);
		return -5;
	}

	ret = content_length;
	curl_easy_cleanup(curl_handle);

	return ret;
}

/*Adapted from - http://curl.haxx.se/libcurl/c/multi-app.html*/
int initialize_curl_handle( CURL ** http_handle_ref, int i, videourl * url, struct myprogress * prog, CURLM *multi_handle)
{
	char range[50];
	char url_now[1500];
	strcpy(url_now, url->url);
	if(metric.playout_buffer_seconds>0)
	{
		sprintf(range, "&range=%ld-%ld",url->range0, url->range1);
		strcat(url_now,range);
	}
	CURL * http_handle;
	if(*http_handle_ref != NULL)
	{
		http_handle = *http_handle_ref;
		curl_multi_remove_handle(multi_handle,http_handle);
	}
	else
	{
		prog->stream = i;
		prog->init = false;
	}
	prog->lastdlbytes = 0;
	prog->lastruntime = 0;

	http_handle = curl_easy_init();
	if(http_handle == NULL)
	{
		fprintf(stderr, "curl_easy_init() failed and returned NULL\n");
		return 0;
	}
	*http_handle_ref = http_handle;

	/* set options */
    if (program_arguments.instantaneous_output)
    	puts(url_now);
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_URL, url_now),i);
	set_ip_version(http_handle, program_arguments.ip_version);
	/* if redirected, tell libcurl to follow redirection */
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle , CURLOPT_FOLLOWLOCATION, 1L),i);
	/* if received 302, follow location  */
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_POSTREDIR, 1L),i);	/* if redirected, tell libcurl to follow redirection */

	/* timeout if transaction is not completed in 5 and a half minutes */
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle , CURLOPT_TIMEOUT, 330),i);
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_REFERER, metric.link),i);
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_SSL_VERIFYPEER, 0),i);
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_3),i);
	/*setting the progress function only when range is not given, otherwise interim results are published for each chunk only*/
	prog->curl = http_handle;
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_NOPROGRESS, 0L),i);
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_PROGRESSFUNCTION, older_progress),i);
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_PROGRESSDATA, prog),i);
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_WRITEFUNCTION, write_data),i);
	my_curl_easy_returnhandler(curl_easy_setopt(http_handle, CURLOPT_WRITEDATA, prog),i);


	/* add the individual transfers */
	CURLMcode mret = curl_multi_add_handle(multi_handle, http_handle);
	if(mret != CURLM_OK)
	{
		fprintf(stderr, "curl_multi_add_handle() failed with %d: %s\n",mret, curl_multi_strerror(mret));
		return 0;
	}
	return 1;

}

void load_streams(videourl url [], struct download_parameters* download_files_values)
{
	for(int i = 0; i < metric.numofstreams; i++)
	{
		url[i].range0=url[i].range1;
		/*If range1 is greater than zero, this is not the first fetch*/
		if(url[i].range1>0)
			++url[i].range0;
			/*First fetch, set playing to 1 for the stream*/
		else
			url[i].playing = 1;
		if(!url[i].playing)
		{
			/*the stream is not playing, move on*/
			continue;
		}
		else
			++(download_files_values->run);

		/*Time to download more, check separately for each stream to decide how much to download. */
		download_files_values->playoutbuffer_len = get_curr_playoutbuf_len_forstream(i);
		if(download_files_values->playoutbuffer_len.tv_sec>metric.playout_buffer_seconds)
			continue;
		size_t range_addition;
		if((metric.playout_buffer_seconds - download_files_values->playoutbuffer_len.tv_sec)>=LEN_CHUNK_MINIMUM)
			range_addition = url[i].bitrate*LEN_CHUNK_MINIMUM;
		else
			range_addition = url[i].bitrate*LEN_CHUNK_FETCH;
		url[i].range1 += range_addition;
#ifdef DEBUG
		printf("Getting next chunk for stream %d with range : %ld - %ld\n", i, url[i].range0, url[i].range1);
#endif
		if(!initialize_curl_handle(download_files_values->handles.http_handle+i, i,url+i,download_files_values->prog+i,
								   download_files_values->handles.multi_handle))
			my_curl_cleanup(download_files_values->prog, download_files_values->handles.multi_handle,
							download_files_values->handles.http_handle, i, CURLERROR,url);
	}
}

void reset_file_descriptors(fd_set* fdread, fd_set* fdwrite, fd_set* fdexcep)
{
	FD_ZERO(fdread);
	FD_ZERO(fdwrite);
	FD_ZERO(fdexcep);
}

int report_curl_error(const char* message, CURLMcode mret, struct download_handles* handles, struct myprogress * prog,
		videourl url[])
{
	fputs(message, stderr);
	fprintf(stderr, "with %d: %s\n", mret,
			curl_multi_strerror(mret));
	return my_curl_cleanup(prog, handles->multi_handle, handles->http_handle,
						   metric.numofstreams, CURLERROR, url);
}

int prepare_file_descriptors(videourl url[], struct download_handles* handles, struct myprogress * prog)
{
	fd_set fdread;
	fd_set fdwrite;
	fd_set fdexcep;
	int maxfd = -1;

	long curl_timeo = -1;

	reset_file_descriptors(&fdread, &fdwrite, &fdexcep);

	/* set a suitable timeout to play around with */
	struct timeval timeout = {
			.tv_sec = 1,
			.tv_usec = 0
	};

	CURLMcode mret = curl_multi_timeout(handles->multi_handle, &curl_timeo);
	if(mret==CURLM_OK)
	{
		if(curl_timeo >= 0) {
			timeout.tv_sec = curl_timeo / 1000;
			if(timeout.tv_sec > 1)
				timeout.tv_sec = 1;
			else
				timeout.tv_usec = (curl_timeo % 1000) * 1000;
		}

		/* get file descriptors from the transfers */
		mret = curl_multi_fdset(handles->multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
	}
	if(mret != CURLM_OK)
		return report_curl_error("curl_multi_timeout()/curl_multi_fdset failed", mret, handles, prog, url);
	/* select() return code */
	int rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
	if (rc == -1)
	{
		fprintf(stderr, "select error occured: %d , %s\n", errno, strerror(errno));
		return my_curl_cleanup(prog, handles->multi_handle, handles->http_handle, metric.numofstreams, CURLERROR,url);
	}
	return 0;
}

int run_multiperform(struct download_handles* handles, videourl url[], int* is_still_running, struct myprogress * prog)
{
	/* timeout or readable/writable sockets */
	CURLMcode mret = curl_multi_perform(handles->multi_handle, is_still_running);
	while (mret ==CURLM_CALL_MULTI_PERFORM)
		mret = curl_multi_perform(handles->multi_handle, is_still_running);

	if(mret != CURLM_OK)
		return report_curl_error("curl_multi_perform() failed", mret, handles, prog, url);
	return 0;
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

int find_handler_index(CURL *easy_handle, CURL **handlers)
{
	for (int idx = 0; idx < metric.numofstreams; idx++)
	{
		if (easy_handle == handlers[idx])
			return idx;
	}
	return -1;
}

void update_stream_progress(videourl *url, int idx, CURL **http_handles, struct myprogress *prog)
{
	if(url[idx].playing)
		update_curl_progress(prog, http_handles, idx);
	double bytes_now;
	if(curl_easy_getinfo(http_handles[idx], CURLINFO_SIZE_DOWNLOAD, &bytes_now)!= CURLE_OK)
	{
		url[idx].playing = 0;
		metric.errorcode = CURLERROR_GETINFO;
	}
	else
	{
		if(bytes_now < url[idx].range1 - url[idx].range0)
		{
#ifdef DEBUG
			printf("Full HTTP transfer completed for stream %d  with status %d\n", idx, msg->data.result);fflush(stdout);
#endif
			url[idx].playing = 0;
		}
	}
}

int process_running(videourl url[], struct download_parameters* download_params)
{
	struct download_handles* handles = &download_params->handles;
	do {
		int result_status = prepare_file_descriptors(url, handles, download_params->prog);
		if (result_status != 0)
			return result_status;
		result_status = run_multiperform(handles, url, &download_params->still_running, download_params->prog);

		if (result_status != 0)
			return result_status;
		CURLMsg *msg; /* for picking up messages with the transfer status */
		int msgs_left; /* how many messages are left */
		while ((msg = curl_multi_info_read(handles->multi_handle, &msgs_left)))
		{
			if (msg->msg == CURLMSG_DONE) {
				/* Find out which handle this message is about */
				int idx = find_handler_index(msg->easy_handle, handles->http_handle);
				if (idx >= 0)
					update_stream_progress(url, idx, handles->http_handle, download_params->prog);
			}
		}
		check_max_test_time();
		if(metric.errorcode)
		{
			download_params->run = 0;
			break;
		}
		/*checkstall should not be called here, more than one TS might be received during one session, it is important that
          we check the status of the playout buffer as soon as a new frame(TS) is received
        checkstall(false);*/
	} while(download_params->still_running);
}

int downloadfiles(videourl url [] )
{
	struct download_parameters download_files_values = {
		.run = metric.numofstreams
	};

	download_files_values.handles.multi_handle = curl_multi_init();
	memzero(download_files_values.handles.http_handle, sizeof(CURL *)*NUMOFSTREAMS);
	/* init a multi stack */
	if(download_files_values.handles.multi_handle == NULL)
	{
		fprintf(stderr, "curl_multi_init() failed and returned NULL\n");
		return CURLERROR;
	}
	download_files_values.prog = malloc(sizeof(struct myprogress [metric.numofstreams]));
	metric.stime = gettimelong();
	while(download_files_values.run) {
		download_files_values.run = 0;
		/*Previous transfers have finished, figure out if we have enough space in the buffer to download 
		  more or should we wait while the buffer empties */
		/*Get the length of the current buffer*/
		download_files_values.playoutbuffer_len = get_curr_playoutbuf_len();
		/*Adjust timetowait based on the length of the playout buffer, we don't care about usecs*/
		if(download_files_values.playoutbuffer_len.tv_sec>metric.playout_buffer_seconds)
		{
			download_files_values.playoutbuffer_len.tv_sec =
					download_files_values.playoutbuffer_len.tv_sec-metric.playout_buffer_seconds;
#ifdef DEBUG
			printf("Time to wait: %d seconds. Sleeping at %ld....", playoutbuffer_len.tv_sec, gettimeshort());
#endif
			select(0, NULL, NULL, NULL, &download_files_values.playoutbuffer_len);
#ifdef DEBUG
			printf("Awake %ld\n", gettimeshort());
#endif
		}

		load_streams(url, &download_files_values);

		if(download_files_values.run == 0)
			break; 
		/* we start some action by calling perform right away */
		CURLMcode mret;
		do {
			mret = curl_multi_perform(download_files_values.handles.multi_handle, &download_files_values.still_running);
		}
		while (mret == CURLM_CALL_MULTI_PERFORM);

		if(mret != CURLM_OK)
		{
			return report_curl_error("curl_multi_perform() failed with", mret, &download_files_values.handles,
							  download_files_values.prog,url);
		}

		process_running(url, &download_files_values);
	}
	return my_curl_cleanup(download_files_values.prog, download_files_values.handles.multi_handle,
						   download_files_values.handles.http_handle, metric.numofstreams, ITWORKED, url);

}
