/*
 * engine_structs.h
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */

#ifndef SRC_ENGINE_STRUCTS_H_
#define SRC_ENGINE_STRUCTS_H_

#include <sys/socket.h>
#include <event.h>
#include <pthread.h>
#include <semaphore.h>

#include "lsquic.h"
#include "connection_params.h"
#include "args_data.h"
#include "stack.h"
#include "cycle_buffer.h"
#include "simple_list.h"
#include "queue.h"
#include "error_report.h"


// standard default value to the library documentation
#define QUIC_IPV4_MAX_PACKET_SIZE 1370
#define QUIC_IPV6_MAX_PACKET_SIZE 1350

int is_debug;

typedef struct event_timer {
	struct event_base* event_base_ref;
	struct event* event_timeout_ref;
	struct event* event_connect_queue_ref;
	//struct event* event_keep_alive_ref;
} event_timer_t;


struct queued_request{
	struct download_request* request;
	struct text* final_url;
	struct timespec start_time;
};

typedef struct quic_engine_shell {
	quic_args program_args;

	lsquic_engine_t* engine_ref;
	struct lsquic_engine_api engine_api;
	struct lsquic_engine_settings engine_settings;

	struct simple_list* connections;

	event_timer_t events;

	struct lsquic_logger_if logger;

	struct queue* request_queue;
	struct queue* free_ports_queue;
	struct primitive_stack* output_stack_ref;
	size_t max_packet_size;
    // streams that have a connection and between initialized and read state
    uint reading_streams_count;
    int is_closed;
} quic_engine_parameters;

void report_engine_error(text_t* error_message, int error_code, quic_engine_parameters* engine, text_t* file, int line,
						 int local_error_code);
void report_stream_error(text_t* error_message, int error_code, stream_parameters_t* stream, text_t* file, int line,
						 int local_error_code);
quic_engine_parameters* extract_stream_engine(stream_parameters_t *stream);
void report_engine_error_with_report(quic_engine_parameters* engine, error_report_t* report);

#endif /* SRC_ENGINE_STRUCTS_H_ */
