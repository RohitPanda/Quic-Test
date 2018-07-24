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

#define _GNU_SOURCE
#include <libavformat/avformat.h>
#include <string.h>
#include <limits.h>

#include "math.h"
#include "youtube-dl.h"
#include "helper.h"
#include "metrics.h"
#include "curlops.h"
#include "getinfo.h"
#include "attributes.h"
#include "network_addresses.h"

metrics metric;
#define PAGESIZE 500000

struct program_arguments program_arguments = {
		.instantaneous_output = false,
		.max_test_time = INT_MAX,
		.buffer_len = LEN_CHUNK_MINIMUM,
		.ip_version = IPvSYSTEM,
		.max_bitrate = INT_MAX,
		.min_test_time = 0,
		.one_bitrate = false,
};

static void signal_handler(int UNUSED(sig))
{
    metric.errorcode = SIGNALHIT;
    metric.etime= gettimelong();
    exit(0);
}

static void mainexit()
{
#ifdef DEBUG
	printf("exiting main\n");
#endif
	if(metric.errorcode==0)
		metric.errorcode = WERSCREWED;
	printvalues();
}

static int init_libraries() {
	CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);

	// silence ffmpeg if it's not required
    if (!program_arguments.instantaneous_output)
        av_log_set_level(AV_LOG_QUIET);

	if(ret != CURLE_OK) {
		return -1;
	}
	//av_register_all();

	return 0;
}

static int destroy_libraries() {
    curl_global_cleanup();
    //av_register_all();

    return 0;
}

static int prepare_exit() {
	if(atexit(mainexit) != 0) {
		return -1;
	}

	if(signal(SIGINT, signal_handler) == SIG_ERR) {
		return -1;
	}

	if(signal(SIGTERM, signal_handler) == SIG_ERR) {
		return -1;
	}

	return 0;
}

static void general_drop_by_default_metrics(metrics *metric)
{
    metric->errorcode=0;
    metric->fail_on_stall = true;
    metric->Tmin0 = -1;
    metric->T0 = -1;
    metric->htime = gettimelong();
    metric->initialprebuftime = -1;
}

static void init_metrics(metrics *metric) {
	memzero(metric, sizeof(*metric));
	general_drop_by_default_metrics(metric);
	metric->ft = NOTSUPPORTED;
	metric->playout_buffer_seconds = program_arguments.buffer_len;
}

static void restart_metrics(metrics *metric) {
    general_drop_by_default_metrics(metric);
	metric->numofstalls = 0;
	metric->totalstalltime = 0;
	metric->TSnow = 0;
	memset(metric->TSlist, 0, sizeof(metric->TSlist));
	metric->TS0 = 0;
}

static size_t write_to_memory(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	static size_t num;

	char *output = userdata;
	size_t bytes = size * nmemb;

	size_t free_space = PAGESIZE - num;
	size_t written = free_space > bytes ? bytes : free_space;

	memcpy(output + num, ptr, written);
	num += written;

	return written;
}

static int download_to_memory(char* url, void *memory) {
	CURL *curl = curl_easy_init();
	if(!curl) {
		return -1;
	}

	if(set_ip_version(curl, program_arguments.ip_version) < 0) {
        if(curl)
            curl_easy_cleanup(curl);
		return -2;
	}

	CURLcode error = 0;
	error |= curl_easy_setopt(curl, CURLOPT_URL, url);
	/* if redirected, tell libcurl to follow redirection */
	error |= curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	/* timeout if no response is received in 30 seconds */
	error |= curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	/* send all data to this function  */
	error |= curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_memory);
	/* we want the data to this memory */
	error |= curl_easy_setopt(curl, CURLOPT_WRITEDATA, memory);
	error |= curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	error |= curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_3);

	if(error) {
        if(curl)
            curl_easy_cleanup(curl);
		return -3;
	}

	/* Perform the request, error will get the return code */
	error = curl_easy_perform(curl);

	/* Check for errors  */
	if(error != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed with %d: %s\n", error,
				 curl_easy_strerror(error));

	long http_code;
	error = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
	if(error || http_code != 200)
	{
        if (curl)
            curl_easy_cleanup(curl);
		return -4;
	}

	if( curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &metric.tcp_first_connection_time_us)!= CURLE_OK)
		metric.tcp_first_connection_time_us = -1;
        else {
		double lookup_time; 
        	if( curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &lookup_time)!= CURLE_OK) {
                	metric.tcp_first_connection_time_us=-1;
                } else {
                	metric.tcp_first_connection_time_us -= lookup_time;
                }
        }
	if(curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &metric.first_full_connection_time_us)!= CURLE_OK)
		metric.first_full_connection_time_us = -1;
    metric.startup = gettimelong() - metric.htime;
    if (curl)
        curl_easy_cleanup(curl);
    return 0;
}

static int extract_media_urls(char youtubelink[]) {
	int ret = 0;

	char *pagecontent = malloc(sizeof(char [PAGESIZE]));
	if(pagecontent == NULL) {
		free(pagecontent);
		return -1;
	}
	memzero(pagecontent, PAGESIZE*sizeof(char));

	if(download_to_memory(youtubelink, pagecontent) < 0) {
		metric.errorcode = FIRSTRESPONSERROR;
		free(pagecontent);
		return -2;
	}

//		TODO:metric.itag needs to be added, updated and printed.
//	TODO:errorcodes for each stream can be different. Add functionality to override errorcode of 1 over the other.

	ret = find_urls(pagecontent);

	free(pagecontent);

	return ret;
}

filetype filetype_from_text(const char* type_str, int is_adaptive)
{
    if (is_adaptive)
    {
        if(metric.numofstreams > 1) {
            if(metric.ft == MP4) {
                return MP4_A;
            } else if(metric.ft == WEBM) {
                return WEBM_A;
            }
        }
    }

    if(strcasestr(metric.url[0].type, "MP4"))
        return MP4;
    else if(strcasestr(metric.url[0].type, "webm"))
        return WEBM;
    else if(strcasestr(metric.url[0].type, "flv"))
        return FLV;
    else if(strcasestr(metric.url[0].type, "3gpp"))
        return TGPP;
}

void init(int argc, char* argv[], char* youtubelink)
{
    if(!set_arguments(argc, argv, youtubelink, &program_arguments))
        exit(EXIT_FAILURE);

    if(prepare_exit() < 0) {
        exit(EXIT_FAILURE);
    }

    if(init_libraries() < 0) {
        exit(EXIT_FAILURE);
    }

    init_metrics(&metric);
}

void check_process_errors(bool* should_countinue_out)
{
	*should_countinue_out = false;
	if(metric.errorcode == ITWORKED && metric.downloadtime[STREAM_VIDEO] < program_arguments.min_test_time) {
		metric.errorcode = TOO_FAST;
	}

	if(metric.fail_on_stall && (metric.errorcode == ERROR_STALL || metric.errorcode == TOO_FAST)) {
		printvalues();
		*should_countinue_out = true;
	}
}

bool is_url_left(const char* url)
{
	return strlen(url) > 0;
}

bool process_adaptive()
{
	int i = 0;
	do {
		restart_metrics(&metric);

		metric.url[0] = metric.adap_videourl[i];
		metric.numofstreams = 1;

		int j = 0;
		do {
			char *vformat = strstr(metric.adap_videourl[i].type, "video/") + strlen("video/");
			char *aformat = strstr(metric.adap_audiourl[j].type, "audio/") + strlen("audio/");

			if(strcmp(vformat, aformat) == 0) {
				metric.url[1] = metric.adap_audiourl[j];
				metric.numofstreams = 2;
				break;
			}
		} while(strlen(metric.adap_audiourl[++j].url) != 0);

		if(metric.url[0].bitrate + metric.url[1].bitrate > program_arguments.max_bitrate) {
			continue;
		}

		metric.ft = filetype_from_text(metric.url[0].type, 1);

		bool are_urls_left = is_url_left(metric.adap_videourl[i + 1].url) && is_url_left(metric.no_adap_url[0].url);
		if(are_urls_left || program_arguments.one_bitrate)
			metric.fail_on_stall = false;

		downloadfiles(metric.url);
		bool should_continue;
		check_process_errors(&should_continue);
		if (should_continue)
			continue;

		return true;
	} while(is_url_left(metric.adap_videourl[++i].url));
	return false;
}

void reset_adaptive_metric()
{
	memset(&metric.url[STREAM_AUDIO], 0, sizeof(metric.url[STREAM_AUDIO]));
	memset(&metric.cdnip[STREAM_AUDIO], 0, sizeof(metric.cdnip[STREAM_AUDIO]));
	metric.downloadtime[STREAM_AUDIO] = 0;
	metric.totalbytes[STREAM_AUDIO] = 0;
	metric.TSlist[STREAM_AUDIO] = 0;
	metric.downloadrate[STREAM_AUDIO] = 0;
	metric.tcp_connection_time_us[STREAM_AUDIO] = 0;
	metric.full_ssl_connection_time_us[STREAM_AUDIO] = 0;
	metric.numofstreams = 1;
}

bool process_nonadaptive()
{
	reset_adaptive_metric();

	int i = 0;
	do {
		restart_metrics(&metric);

		metric.url[0] = metric.no_adap_url[i];

		if(metric.url[0].bitrate > program_arguments.max_bitrate) {
			continue;
		}

		metric.ft = filetype_from_text(metric.url[0].type, 0);

		if(is_url_left(metric.no_adap_url[i + 1].url) || program_arguments.one_bitrate)
			metric.fail_on_stall = false;

		downloadfiles(metric.url);

		bool should_continue;
		check_process_errors(&should_continue);
		if (should_continue)
			continue;

		return true;
	} while(is_url_left(metric.no_adap_url[++i].url));
	return false;
}

int main(int argc, char* argv[])
{
	char youtubelink[MAXURLLENGTH]="http://www.youtube.com/watch?v=j8cKdDkkIYY";

	init(argc, argv, youtubelink);

	strncpy(metric.link, youtubelink, MAXURLLENGTH - 1);
	if(extract_media_urls(youtubelink) < 0) {
		exit(EXIT_FAILURE);
	}

	if (!process_adaptive())
		process_nonadaptive();

	if(metric.errorcode == 403) {
		return 148;
	} else if(metric.errorcode == TOO_FAST) {
		return 149;
	} else {
		return 0;
	}
	destroy_libraries();
}
