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

#include "quic_downloader.h"
#include "math.h"
#include "youtube-dl.h"
#include "helper.h"
#include "metrics.h"
#include "download_ops.h"
#include "getinfo.h"
#include "attributes.h"
#include "network_addresses.h"
#include "timespec_convertor.h"

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
		.ports_count = 0,
		.quic_version = "Q039",
		.is_connection_level_debug = false,
		.one_stream_per_connection = false
};

quic_args quic_arguments;

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

#define TIMEOUT_MS 330000
#define default_ip_version IPv6



static int init_quic_client()
{
    quic_arguments.is_debug = program_arguments.is_connection_level_debug;
    quic_arguments.timeout_ms = TIMEOUT_MS;
    // max streams takes into account redirection cases and 1 stream for web page
    quic_arguments.max_streams = NUMOFSTREAMS * 2 + 1;
    if (program_arguments.ip_version == IPvSYSTEM)
    {
        enum ip_support local_ip_support = get_ip_version_support();
        if (local_ip_support == IP_SUPPORT_NONE || local_ip_support == IP_SUPPORT_BOTH)
            quic_arguments.is_ipv4 = default_ip_version == IPv4;
    } else quic_arguments.is_ipv4 = program_arguments.ip_version == IPv4;
    quic_arguments.port_count = program_arguments.ports_count;
    quic_arguments.local_port_numbers = program_arguments.port_numbers;

    quic_arguments.quic_ver_str_size = sizeof(program_arguments.quic_version) - 1;
    quic_arguments.quic_ver_str = program_arguments.quic_version;

    quic_arguments.use_prev_conn_data = 0;
    quic_arguments.is_one_conn_per_stream = program_arguments.one_stream_per_connection;
    if (create_client(&quic_client, &quic_arguments) != 0)
    {
        printf("quic client was not created");
        return -1;
    }
}

static int init_libraries() {
    if (!program_arguments.instantaneous_output)
        av_log_set_level(AV_LOG_QUIET);

    if (init_quic_client() == -1)
        return -1;

	return 0;
}

static int destroy_libraries() {
    free(program_arguments.port_numbers);

    destroy_client(quic_client);
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

static int download_to_memory(struct download_request *request) {
	request->buffer.used_size = 0;
	request->max_write_size = 0;
	request->write_argument = NULL;
	request->on_write = NULL;
	request->header_only = 0;
	int ret = 0;

	start_downloading(quic_client, request, 1);
	struct output_data* output_data = get_download_result(quic_client);

	if(output_data->error_code || output_data->http_code != 200)
	{
		if(output_data->error_code)
			fprintf(stderr, "page downloading failed: %s\n", output_data->error_str);
		destroy_output_data_container(output_data);
		destroy_client(quic_client);
		return -4;
	}
	metric.first_website_connection_time_s =  timespec_to_seconds(&output_data->connection_time);
	metric.startup = gettimelong() - metric.htime;

	destroy_output_data_container(output_data);

	return ret;
}

static int extract_media_urls(char youtubelink[]) {
	int ret = 0;
	struct download_request request;
	request.buffer.buffer = malloc(sizeof(char [PAGESIZE]));
	request.buffer.allocated_size = PAGESIZE;
	if(request.buffer.buffer == NULL) {
		free(request.buffer.buffer);
		return -1;
	}
	memzero(request.buffer.buffer, PAGESIZE*sizeof(char));

	request.url_request.url_data = youtubelink;
	request.url_request.url_len = strlen(youtubelink);

	if(download_to_memory(&request) < 0) {
		metric.errorcode = FIRSTRESPONSERROR;
		free(request.buffer.buffer);
		return -2;
	}

//		TODO:metric.itag needs to be added, updated and printed.
//	TODO:errorcodes for each stream can be different. Add functionality to override errorcode of 1 over the other.

	if (find_urls(request.buffer.buffer) < 0)
    {
        free(request.buffer.buffer);
        return -3;
    }

	return 0;
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
            char* found_pointer = strstr(metric.adap_videourl[i].type, "video/");
            char *vformat = found_pointer + strlen("video/");
            found_pointer = strstr(metric.adap_audiourl[j].type, "audio/");
			char *aformat = found_pointer + strlen("audio/");

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

void reset_audio_metric()
{
	memset(&metric.url[STREAM_AUDIO], 0, sizeof(metric.url[STREAM_AUDIO]));
	memset(&metric.http_ver[STREAM_AUDIO], 0, sizeof(metric.http_ver[STREAM_AUDIO]));
	memset(&metric.quic_ver[STREAM_AUDIO], 0, sizeof(metric.quic_ver[STREAM_AUDIO]));
	memset(&metric.cdnip[STREAM_AUDIO], 0, sizeof(metric.cdnip[STREAM_AUDIO]));
	metric.downloadtime[STREAM_AUDIO] = 0;
	metric.totalbytes[STREAM_AUDIO] = 0;
	metric.TSlist[STREAM_AUDIO] = 0;
	metric.downloadrate[STREAM_AUDIO] = 0;
	metric.connection_time_s[STREAM_AUDIO] = 0;
	metric.numofstreams = 1;
}

bool process_nonadaptive()
{
	reset_audio_metric();

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

	strncpy(metric.link, youtubelink, MAXURLLENGTH-1);
	if(extract_media_urls(youtubelink) < 0) {
		exit(EXIT_FAILURE);
	}

	if (!process_adaptive())
		process_nonadaptive();

    destroy_libraries();

	if(metric.errorcode == 403) {
		return 148;
	} else if(metric.errorcode == TOO_FAST) {
		return 149;
	} else {
		return 0;
	}

}
