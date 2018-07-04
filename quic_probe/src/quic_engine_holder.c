/*
 * quic_engine_holder.c
 *
 *  Created on: Apr 13, 2018
 *      Author: sergei
 */


#include "quic_engine_holder.h"

#include <stdio.h>
#include <lsquic.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>

#include "address_finder.h"
#include "connection_init_handler.h"
#include "connection_read_handler.h"
#include "connection_write_handler.h"
#include "socket_handler.h"
#include "connection_params.h"
#include "socket_buffer.h"
#include "connection_establisher.h"
#include "event_handlers.h"

static const struct lsquic_stream_if http_client_if = {
    .on_new_conn            = http_client_on_new_conn,
    .on_conn_closed         = http_client_on_conn_closed,
    .on_new_stream          = http_client_on_new_stream,
    .on_read                = http_client_on_read,
    .on_write               = http_client_on_write,
    .on_close               = http_client_on_stream_close,
	.on_hsk_done			= http_client_on_connection_handshake
};

#define QUIC_PORT_NUMBER 443

extern int is_debug;

int printlog(void *logger_ctx, const char *fmt, va_list args)
{
	return printf(fmt, args);
}

static int
file_vprintf (void *ctx, const char *fmt, va_list ap)
{
    return vfprintf((FILE *) ctx, fmt, ap);
}

void allocate_output(quic_engine_parameters* quic_engine_ref)
{
	quic_engine_ref->output_stack_ref = init_stack();
}

#define DEFAULT_MAX_STREAMS 10 * 100
#define DEFAULT_TIMEOUT_US 30 * 1000 * 1000


void init_free_ports_queue(struct queue** port_queue_ref, uint* ports, unsigned short ports_count){
    *port_queue_ref = init_queue();
    for (int i = 0; i < ports_count; i++) {
        queue_put_data(*port_queue_ref, (void*) (ports + i));
    }

}

void set_lsquic_settings(struct lsquic_engine_settings* settings, unsigned char max_streams, size_t timeout_ms) {
    settings->es_max_streams_in = max_streams == 0? DEFAULT_MAX_STREAMS : max_streams;
    //settings->es_handshake_to = timeout_ms == 0? DEFAULT_TIMEOUT_US : timeout_ms * 1000;
    // try handshake to 0 to end stream by timeout event
    settings->es_handshake_to = 0;
}

int init_engine(quic_engine_parameters* quic_engine_ref)
{
	allocate_output(quic_engine_ref);
	quic_engine_ref->max_packet_size = quic_engine_ref->program_args.is_ipv4? QUIC_IPV4_MAX_PACKET_SIZE :
			QUIC_IPV6_MAX_PACKET_SIZE;
	set_lsquic_settings(&quic_engine_ref->engine_settings, quic_engine_ref->program_args.max_streams,
                        quic_engine_ref->program_args.timeout_ms);
	lsquic_engine_init_settings(&quic_engine_ref->engine_settings, LSENG_HTTP);
	quic_engine_ref->engine_api.ea_settings = &quic_engine_ref->engine_settings;
	quic_engine_ref->engine_api.ea_stream_if = &http_client_if;
	quic_engine_ref->engine_api.ea_stream_if_ctx = (void*)quic_engine_ref;
	quic_engine_ref->engine_api.ea_packets_out = on_packets_out;
	quic_engine_ref->engine_api.ea_packets_out_ctx = (void*)quic_engine_ref;
	quic_engine_ref->engine_api.ea_pmi = NULL;
	quic_engine_ref->engine_api.ea_pmi_ctx = NULL;
	quic_engine_ref->engine_settings.es_versions = 1 << lsquic_str2ver(quic_engine_ref->program_args.quic_ver_str,
			quic_engine_ref->program_args.quic_ver_str_size);
	lsquic_global_init(LSQUIC_GLOBAL_CLIENT);
	quic_engine_ref->engine_ref = lsquic_engine_new(LSENG_HTTP, &quic_engine_ref->engine_api);

	// flag that engine was created
	quic_engine_ref->is_closed = 0;
	// create event base and hang 2 events upon
	quic_engine_ref->events.event_base_ref = event_base_new();
	quic_engine_ref->events.event_timeout_ref = event_new(quic_engine_ref->events.event_base_ref, -1, 0, timer_handler,
			(void*)quic_engine_ref);
	quic_engine_ref->events.event_connect_queue_ref = event_new(quic_engine_ref->events.event_base_ref, -1, 0,
			connect_event_handler, (void*)quic_engine_ref);
	quic_engine_ref->reading_streams_count = 0;

    init_free_ports_queue(&quic_engine_ref->free_ports_queue, quic_engine_ref->program_args.local_port_numbers,
                          quic_engine_ref->program_args.port_count);
	quic_engine_ref->connections = init_simple_list();
	quic_engine_ref->request_queue = init_queue();

	if (is_debug)
	{
        quic_engine_ref->logger.vprintf = file_vprintf;
        lsquic_logger_init(&quic_engine_ref->logger, stderr, LLTS_HHMMSSMS);
        lsquic_set_log_level("debug");
	}

	return 0;
}

void destroy_events(event_timer_t* events)
{
	event_free(events->event_timeout_ref);
	event_free(events->event_connect_queue_ref);
	event_base_free(events->event_base_ref);
}

void destroy_engine(quic_engine_parameters* quic_engine_ref)
{
	//destroy_connections(quic_engine_ref);
	lsquic_engine_destroy(quic_engine_ref->engine_ref);

    destroy_queue(quic_engine_ref->request_queue);
    destroy_queue(quic_engine_ref->free_ports_queue);
    destroy_events(&quic_engine_ref->events);

	destroy_stack(quic_engine_ref->output_stack_ref);
	lsquic_global_cleanup();
	quic_engine_ref->is_closed = 1;
}
