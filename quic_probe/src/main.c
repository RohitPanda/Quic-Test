/*
 * main.c
 *
 *  Created on: Apr 4, 2018
 *      Author: sergei
 */


#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "../include/quic_downloader.h"

extern int is_debug;

void write_to_file(char* start_pointer, size_t size, const char* file_name)
{
    FILE* file = fopen(file_name ,"w");
    for (; size > 0; size--) {
        size_t len = strlen(start_pointer);
        fputc(*start_pointer, file);
        start_pointer++;
    }
    fclose(file);
}

void read_result(struct quic_engine_shell* client, const char* file_name, double* total_time_out){
	struct output_data* result = get_download_result(client);

    if (result == NULL)
        return;

    write_to_file(result->request_args->buffer.buffer, result->request_args->buffer.used_size, file_name);

	if (result->error_code)
	    printf("error: %s, url: %s\n", result->error_str, result->final_url.url_data);
	else
    {
        *total_time_out += result->download_time.tv_sec + result->connection_time.tv_nsec / 1000000000.0;
        printf("%s\n%s was written : conn time %ld us, download time: %ld us, bytes: %ld, %s, ip: %s\n",
               result->request_args->url_request.url_data,
               file_name, result->connection_time.tv_sec * 1000000 + result->connection_time.tv_nsec / 1000,
               result->download_time.tv_sec * 1000000 + result->download_time.tv_nsec / 1000,
               result->request_args->buffer.used_size, result->http_header, result->used_ip);
    }

	if (result != NULL)
		destroy_output_data_container(result);
}

#define BUFFER_SIZE 10 * 1024 * 1024
#define QUIC_VERSION "Q043"

int main()
{
    uint ports[] = {3039};
	quic_args quic_args_ref =
	{
		.is_ipv4 = 1,
		.quic_ver_str = QUIC_VERSION,
		.quic_ver_str_size = strlen(QUIC_VERSION),
		.timeout_ms = 60000,
		.use_prev_conn_data = 0,
		.local_port_numbers = ports,
        .port_count = 1,
		.max_streams = 10,
        .is_debug = 0,
        .is_one_conn_per_stream = 0

	};
    struct quic_engine_shell* quic_engine_ref;
    int is_created = create_client(&quic_engine_ref, &quic_args_ref);
    if(is_created == -1)
        exit(-1);
    double total_time_s = 0;
//    struct download_request download_requests4[2];
//    download_requests4[0].url_request.url_data = "https://www.youtube.com";
//    download_requests4[0].url_request.url_len = strlen(download_requests4[0].url_request.url_data);
//    download_requests4[0].on_write = NULL;
//
//
//    download_requests4[1].url_request.url_data = "https://www.youtube.com/watch?v=6LAH3qE73lE";
//    download_requests4[1].url_request.url_len = strlen(download_requests4[1].url_request.url_data);
//    download_requests4[1].on_write = NULL;
//
//
//    download_requests4[0].buffer.buffer = malloc(BUFFER_SIZE);
//    download_requests4[0].buffer.allocated_size = BUFFER_SIZE;
//    download_requests4[0].buffer.used_size = 0;
//
//    download_requests4[1].buffer.buffer = malloc(BUFFER_SIZE);
//    download_requests4[1].buffer.allocated_size = BUFFER_SIZE;
//    download_requests4[1].buffer.used_size = 0;
//
//    start_downloading(quic_engine_ref, &download_requests4[0], 1);
//    read_result(quic_engine_ref, "youtube_result.txt");
//
//    start_downloading(quic_engine_ref, &download_requests4[1], 1);
//    read_result(quic_engine_ref, "youtube_result2.txt");
//    //read_result(quic_engine_ref, "google_general.txt");
//
//    download_requests4[0].url_request.url_data = "https://www.youtube.com/watch?v=NMGDnPYopYQ3";
//    download_requests4[0].url_request.url_len = strlen(download_requests4[0].url_request.url_data);
//
//    download_requests4[1].url_request.url_data = "https://www.youtube.com/watch?v=znUg17W526o";
//    download_requests4[1].url_request.url_len = strlen(download_requests4[1].url_request.url_data);
//
//    download_requests4[0].buffer.used_size = 0;
//    download_requests4[1].buffer.used_size = 0;
//
//    start_downloading(quic_engine_ref, &download_requests4[0], 1);
//    read_result(quic_engine_ref, "youtube_result3.txt");
//
//    start_downloading(quic_engine_ref, &download_requests4[1], 1);
//    read_result(quic_engine_ref, "youtube_result4.txt");
//    //read_result(quic_engine_ref, "google_general.txt");
//
//    free(download_requests4[0].buffer.buffer);
//    free(download_requests4[1].buffer.buffer);
//
//    struct download_request download_requests3[1];
//    download_requests3[0].url_request.url_data = "http://google.com";
//    download_requests3[0].url_request.url_len = strlen(download_requests3[0].url_request.url_data);
//    download_requests3[0].on_write = NULL;
//
//
//    download_requests3[0].buffer.buffer = malloc(BUFFER_SIZE);
//    download_requests3[0].buffer.allocated_size = BUFFER_SIZE;
//    download_requests3[0].buffer.used_size = 0;
//
//    start_downloading(quic_engine_ref, download_requests3, 1);
//
//    read_result(quic_engine_ref, "single_result.txt");
//
//    free(download_requests3[0].buffer.buffer);
//
//	struct download_request download_requests[2];
//	download_requests[0].url_request.url_data = "https://www.youtube.com/watch?v=rjp-JZif59M";
//	download_requests[0].url_request.url_len = strlen(download_requests[0].url_request.url_data);
//    download_requests[0].on_write = NULL;
//
//	// test redirect feature last p is wrong url
//	download_requests[1].url_request.url_data = "https://www.youtube.com/watch?v=3xmH2GxduTAp";
//	download_requests[1].url_request.url_len = strlen(download_requests[1].url_request.url_data);
//    download_requests[1].on_write = NULL;
//
//
//	download_requests[0].buffer.buffer = malloc(BUFFER_SIZE);
//	download_requests[0].buffer.allocated_size = BUFFER_SIZE;
//	download_requests[0].buffer.used_size = 0;
//	download_requests[1].buffer.buffer = malloc(BUFFER_SIZE);
//	download_requests[1].buffer.allocated_size = BUFFER_SIZE;
//	download_requests[1].buffer.used_size = 0;
//
//    start_downloading(quic_engine_ref, download_requests, 2);
//
//    read_result(quic_engine_ref, "youtube_result5.txt");
//	read_result(quic_engine_ref, "youtube_result6.txt");
//
//    free(download_requests[0].buffer.buffer);
//    free(download_requests[1].buffer.buffer);
    int times = 30;

    for(int i = 0; i < 30; i++)
    {
        struct download_request download_requests2[2];
        download_requests2[0].url_request.url_data = "https://www.youtube.com/";
        download_requests2[0].url_request.url_len = strlen(download_requests2[0].url_request.url_data);
        download_requests2[0].on_write = NULL;
        download_requests2[0].header_only = 1;

        // test timeout
        download_requests2[1].url_request.url_data = "http://www.google.com";
        download_requests2[1].url_request.url_len = strlen(download_requests2[1].url_request.url_data);
        download_requests2[1].on_write = NULL;
        download_requests2[1].header_only = 0;


        download_requests2[0].buffer.buffer = malloc(BUFFER_SIZE);
        download_requests2[0].buffer.allocated_size = BUFFER_SIZE;
        download_requests2[0].buffer.used_size = 0;
        download_requests2[1].buffer.buffer = malloc(BUFFER_SIZE);
        download_requests2[1].buffer.allocated_size = BUFFER_SIZE;
        download_requests2[1].buffer.used_size = 0;

        start_downloading(quic_engine_ref, download_requests2, 1);

        read_result(quic_engine_ref, "video_quic.txt", &total_time_s);
        //read_result(quic_engine_ref, "audio_quic.txt", &total_time_s);

        free(download_requests2[0].buffer.buffer);
        free(download_requests2[1].buffer.buffer);
    }

    destroy_client(quic_engine_ref);
	printf("Avg time %f\n", total_time_s / times);

	return 0;
}
