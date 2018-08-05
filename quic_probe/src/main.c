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
#include <bits/time.h>
#include <time.h>

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
    printf("Speed:%ld\n", (size_t)(result->request_args->buffer.used_size / (result->download_time.tv_sec + result->download_time.tv_nsec / 1000000000.0)));
    //write_to_file(result->request_args->buffer.buffer, result->request_args->buffer.used_size, file_name);

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
#define QUIC_VERSION "Q039"


struct time_vals{
    size_t downloaded_size;
    size_t last_time;
    struct timespec start_time;
};

size_t get_time_diff(struct timespec start_time) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    size_t nano_secs;
    if (start_time.tv_nsec > now.tv_nsec) {
        nano_secs = now.tv_nsec + 1000000000LL - start_time.tv_nsec;
        now.tv_sec--;
    } else
        nano_secs = now.tv_nsec - start_time.tv_nsec;

    return (now.tv_sec - start_time.tv_sec) * 1000000000LL + nano_secs;
}


size_t write_callback(void* data, size_t size, void *userdata) {
    struct time_vals* vals = (struct time_vals*)userdata;
    size_t time_passed = get_time_diff(vals->start_time);
    vals->downloaded_size += size;
    if (time_passed != 0)
        printf("%ld; %ld\n", time_passed, (size_t)(vals->downloaded_size / (time_passed / 1000000000.0)));
    vals->last_time = time_passed;
    return size;
}


int main(int argc, char **argv)
{
    uint ports[] = {3089, 3090, 3091};
	quic_args quic_args_ref =
    {
        .is_ipv4 = 1,
		.quic_ver_str = QUIC_VERSION,
		.quic_ver_str_size = strlen(QUIC_VERSION),
		.timeout_ms = 60000 * 10,
		.use_prev_conn_data = 0,
		.local_port_numbers = ports,
        .port_count = sizeof(ports)/sizeof(*ports),
		.max_streams = 10,
        .is_debug = 0,
        .is_one_conn_per_stream = 0

	};
    struct quic_engine_shell* quic_engine_ref;
    int is_created = create_client(&quic_engine_ref, &quic_args_ref);
    if(is_created == -1)
        exit(-1);
    double total_time_s = 0;

    int times = 10;

    for(int i = 0; i < times; i++)
    {
        struct time_vals vals = {
                .last_time = 0,
                .downloaded_size = 0
        };

        struct download_request download_requests2[2];
        download_requests2[0].url_request.url_data = argv[1];
        download_requests2[0].url_request.url_len = strlen(download_requests2[0].url_request.url_data);
        download_requests2[0].on_write = write_callback;
        download_requests2[0].write_argument = &vals;
        download_requests2[0].header_only = 0;

        // test timeout
//        download_requests2[1].url_request.url_data = "http://www.google.com";
//        download_requests2[1].url_request.url_len = strlen(download_requests2[1].url_request.url_data);
//        download_requests2[1].on_write = NULL;
//        download_requests2[1].header_only = 0;


        download_requests2[0].buffer.buffer = NULL;
        download_requests2[0].buffer.allocated_size = 0;
        download_requests2[0].buffer.used_size = 0;
//        download_requests2[1].buffer.buffer = malloc(BUFFER_SIZE);
//        download_requests2[1].buffer.allocated_size = BUFFER_SIZE;
//        download_requests2[1].buffer.used_size = 0;

        start_downloading(quic_engine_ref, download_requests2, 1);
        clock_gettime(CLOCK_MONOTONIC, &vals.start_time);

        read_result(quic_engine_ref, "video_quic.txt", &total_time_s);
        //read_result(quic_engine_ref, "audio_quic.txt", &total_time_s);

//        free(download_requests2[0].buffer.buffer);
//        free(download_requests2[1].buffer.buffer);
    }

    destroy_client(quic_engine_ref);
	printf("Avg time %f\n", total_time_s / times);

	return 0;
}
