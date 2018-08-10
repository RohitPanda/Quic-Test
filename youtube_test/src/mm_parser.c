/*
 *
 *      Year   : 2013-2017
 *      Author : Cristian Morales Vega
 *               Saba Ahsan
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

#include "mm_parser.h"
#include "metrics.h"
#include "youtube-dl.h"
#include "attributes.h"
#include "download_ops.h"
#include <stdint.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <regex.h>

extern metrics metric;

#define DIV_ROUND_CLOSEST(x, divisor)(                  \
{                                                       \
        typeof(divisor) __divisor = divisor;            \
        (((x) + ((__divisor) / 2)) / (__divisor));      \
}                                                       \
)

#define SEC2PICO UINT64_C(1000000000000)
//#define SEC2NANO 1000000000
#define SEC2MILI 1000

static int read_packet(void *opaque, uint8_t *buf, int buf_size) {
	struct myprogress *prog = ((struct myprogress *) opaque);
	sem_post(&prog->ffmpeg_awaits_mutex);
	sem_wait(&prog->data_arrived_mutex);
	if (prog->bytes_avail == 0)
		return AVERROR_EOF;
	assert(prog->bytes_avail <= (size_t)buf_size);
	memcpy(buf, prog->mm_parser_buffer, prog->bytes_avail);
	return prog->bytes_avail;
}

unsigned long long calc_tb(int num, int den){

	if (num > (int) (UINT64_MAX / SEC2PICO)) {
		exit(EXIT_FAILURE);
	}
	return DIV_ROUND_CLOSEST(num * SEC2PICO, den);
}

void check_frame_metrics(AVPacket* pkt, uint64_t* ts_list_metrics, unsigned long long tb){
	if (pkt->dts > 0) {
		/*SA-102014. metric.TSnow is the overall timestamp for which both audio and video packets have been received,
          it should not be changed for video frames only. This would not affect the flow when the video is received
		slower than the audio, but if it's the other way around, then the results would be inaccurate
        metric.TSnow = (pkt.dts * vtb) / (SEC2PICO / SEC2MILI);*/
		*ts_list_metrics = (pkt->dts * tb) / (SEC2PICO / SEC2MILI);
		/*SA-10214- checkstall should be called after the TS is updated for each stream, instead of when new packets
          arrive, this ensures that we know exactly what time the playout would stop and stall would occur*/
		checkstall(false);
	}
}


void mm_parser(void *arg) {
	struct myprogress *prog = ((struct myprogress *) arg);

	void *buff = av_malloc(CURL_MAX_WRITE_SIZE);

	AVIOContext *avio = avio_alloc_context(buff, CURL_MAX_WRITE_SIZE, 0,
										   prog, read_packet, NULL, NULL);
	if (avio == NULL) {
		exit(EXIT_FAILURE);
	}

	AVFormatContext *fmt_ctx = avformat_alloc_context();
	if (fmt_ctx == NULL) {
		exit(EXIT_FAILURE);
	}
	fmt_ctx->url = "video file";
	fmt_ctx->pb = avio;
	fmt_ctx->error_recognition = 1;

	int ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);

	if (ret < 0) {
	    char error[500];
	    av_strerror(ret, error, 500);
	    printf("ffpeg error:%s\n", error);
		exit(EXIT_FAILURE);
	}
	avformat_find_stream_info(fmt_ctx, NULL);

	int videoStreamIdx = -1;
	int audioStreamIdx = -1;
	for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {

		if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStreamIdx = i;
		} else if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audioStreamIdx = i;
		}
	}

	unsigned long long tb[2];// video, audio
	if (videoStreamIdx != -1) {
		tb[STREAM_VIDEO] = calc_tb(fmt_ctx->streams[videoStreamIdx]->time_base.num,
								   fmt_ctx->streams[videoStreamIdx]->time_base.den);
	}
	if (audioStreamIdx != -1) {
		tb[STREAM_AUDIO] = calc_tb(fmt_ctx->streams[audioStreamIdx]->time_base.num,
								   fmt_ctx->streams[audioStreamIdx]->time_base.den);
	}

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	while (av_read_frame(fmt_ctx, &pkt) >= AVERROR_EOF) {
		if (pkt.stream_index == videoStreamIdx) {
			check_frame_metrics(&pkt, &metric.TSlist[STREAM_VIDEO], tb[STREAM_VIDEO]);
		} else if (pkt.stream_index == audioStreamIdx) {
			check_frame_metrics(&pkt, &metric.TSlist[STREAM_AUDIO], tb[STREAM_AUDIO]);
		}
		av_packet_unref(&pkt);
	}
	avformat_close_input(&fmt_ctx);
	avformat_free_context(fmt_ctx);
	av_free(avio);

	sem_destroy(&prog->data_arrived_mutex);
	sem_destroy(&prog->ffmpeg_awaits_mutex);

	return;
}

struct timeval get_curr_playoutbuf_len_for_ts(uint64_t ts)
{
	/*check if playout is stalled for any reason, return full length of buffer if so. */
	struct timeval buffertime;
	/*check if playout is stalled for any reason, return full length of buffer if so. */
	if(metric.Tmin<0)
	{
		buffertime.tv_sec=0;
		buffertime.tv_usec=0;
	}
	else
	{
		long long timenow = gettimelong();
		double leninusec =  ((((double)(ts - metric.TS0)*1000)-(double)(timenow-metric.Tmin)));
		buffertime.tv_sec = (long)(leninusec/1000000);
		buffertime.tv_usec = leninusec - (buffertime.tv_sec*1000000);
	}
	return buffertime;
}

/*If the function returns a negative value, it means there is space in the buffer, otherwise it indicates 
the over utilization of the buffer in seconds*/
struct timeval get_curr_playoutbuf_len()
{
	return get_curr_playoutbuf_len_for_ts(metric.TSnow);
}


struct timeval get_curr_playoutbuf_len_forstream(int i)
{
	return get_curr_playoutbuf_len_for_ts(metric.TSlist[i]);
}

