/*
 * connection_establisher.c
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */

#include "connection_establisher.h"

#include <lsquic.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <event.h>
#include <string.h>
#include <arpa/inet.h>

#include "../include/args_data.h"
#include "address_finder.h"
#include "socket_handler.h"
#include "simple_list.h"
#include "event_handlers.h"


int occupy_port(connection_parameters* connection, struct queue* free_ports_queue)
{
    struct socket_shell* socket_shell = &connection->socket_data;
    quic_engine_parameters* engine = (quic_engine_parameters*) connection->quic_engine_params_ref;
    uint* port_ref = queue_pull_data(free_ports_queue);
    // assign port address automatically
    int port_number = 0;
    if (port_ref != NULL)
        port_number = *port_ref;
    error_report_t* error = NULL;
    if (open_socket(engine->program_args.is_ipv4, &connection->local_addr, &socket_shell->socket_id,
                     &socket_shell->socket_buffer_size, port_number, &error) == -1)
    {
        report_engine_error_with_report(engine, error);
        return -1;
    }
    socket_shell->port_number = port_ref;
    int max_packet_size = engine->program_args.is_ipv4? QUIC_IPV4_MAX_PACKET_SIZE : QUIC_IPV6_MAX_PACKET_SIZE;
    init_socket_buffer(&socket_shell->socket_buffer, socket_shell->socket_id, max_packet_size,
                       socket_shell->socket_buffer_size);

    socket_shell->read_event = event_new(engine->events.event_base_ref, socket_shell->socket_id, EV_READ|EV_PERSIST,
            read_socket_event, connection);
    event_add(socket_shell->read_event, NULL);

    return 0;
}


connection_parameters* create_connection(parsed_url_t* parsed_url, int is_ipv4, quic_engine_parameters* engine_ref)
{
    connection_parameters* connection = malloc(sizeof(connection_parameters));
    connection->hostname = copy_text(parsed_url->hostname);
    connection->stream_list = init_simple_list();
    error_report_t* error;
    if (get_sockaddr(parsed_url->hostname->text, &connection->destination_addr, is_ipv4, &error) == -1)
    {
        report_engine_error_with_report(engine_ref, error);
        return NULL;
    }
    connection->ip = sockaddrstorage_to_ip_string(&connection->destination_addr);
    connection->handshake_time.tv_sec = 0;
    connection->handshake_time.tv_nsec = 0;
    connection->quic_engine_params_ref = (void*) engine_ref;
    if (occupy_port(connection, engine_ref->free_ports_queue) == -1)
        return NULL;
    return connection;
}

stream_parameters_t* create_connection_stream(parsed_url_t* parsed_url)
{
    stream_parameters_t* stream_ref = malloc(sizeof(stream_parameters_t));
    stream_ref->is_header_sent = 0;
    stream_ref->is_created = 0;
    stream_ref->path = copy_text(parsed_url->path);
    stream_ref->start_time.tv_sec = 0;
    stream_ref->start_time.tv_nsec = 0;
    return stream_ref;
}

void prepare_stream_output(stream_parameters_t* stream, struct download_request* request, text_t* url)
{
    if (request->max_write_size <= 0)
        request->max_write_size = SIZE_MAX;
    if(request->buffer.used_size != 0)
        request->buffer.used_size = 0;

    stream->data_pointer = init_output_data_container(url->text, url->size);
    stream->data_pointer->request_args = request;

    memset(&stream->data_pointer->http_header, 0, sizeof(stream->data_pointer->http_header));

    stream->data_ref = request->buffer.buffer;
    stream->buffer_left = request->buffer.allocated_size;
}

connection_parameters* open_new_connection(quic_engine_parameters *quic_engine_ref, parsed_url_t *parsed_url,
                                           connection_parameters *connection_shell)
{
    if (connection_shell == NULL)
        return NULL;
    insert_list_element(quic_engine_ref->connections, 0, (void*)connection_shell);
    // set connection start time
    clock_gettime(CLOCK_MONOTONIC_RAW, &connection_shell->start_time);

    connection_shell->connection_ref = lsquic_engine_connect(quic_engine_ref->engine_ref,
            (struct sockaddr*)&connection_shell->local_addr, (struct sockaddr*)&connection_shell->destination_addr,
            (void*)connection_shell, (lsquic_conn_ctx_t*)connection_shell, connection_shell->hostname->text,
                                                             quic_engine_ref->max_packet_size);
    return connection_shell;
}

void charge_request(quic_engine_parameters* engine, struct download_request* request, struct text* final_url,
        struct timespec start_time)
{
    struct queued_request* queued_request = malloc(sizeof(struct queued_request));
    queued_request->final_url = final_url;
    queued_request->start_time = start_time;
    queued_request->request = request;
    queue_put_data(engine->request_queue, (void*)queued_request);
    event_active(engine->events.event_connect_queue_ref, 0, 1);
}

void connect_event_handler (int fd, short what, void *arg)
{
    quic_engine_parameters* engine = (quic_engine_parameters*) arg;
    while (engine->request_queue->length > 0)
    {
        struct queued_request* request = (struct queued_request*)queue_pull_data(engine->request_queue);
        quic_connect(engine, request->request, request->final_url);
        destroy_text(request->final_url);
        free(request);
    }
    lsquic_engine_process_conns(engine->engine_ref);
}

/*
 * destroys parsed url text inside if it was not destroyed before
 * made on optimization purpose to decrease the number of memory allocation calls
 */
void try_destroy_parsed_url(parsed_url_t *parsed_url, int *is_destroyed)
{
    if (!*is_destroyed)
    {
        destroy_text(parsed_url->hostname);
        destroy_text(parsed_url->path);
    }
    *is_destroyed = 1;
}

int quic_connect(quic_engine_parameters* quic_engine_ref, struct download_request* request, text_t* url)
{
    if (quic_engine_ref->is_closed)
    {
        printf("engine was closed");
        return -1;
    }

    int is_text_destroyed = 0;
    error_report_t* error_report;
    parsed_url_t parsed_url;
    if (parse_url(url->text, &parsed_url, &error_report) == -1)
        return -1;

    stream_parameters_t* new_stream_ref = create_connection_stream(&parsed_url);
    prepare_stream_output(new_stream_ref, request, url);

    connection_parameters* connection =  find_connection(quic_engine_ref->connections, parsed_url.hostname->text,
                        quic_engine_ref->program_args.is_one_conn_per_stream);
    if (connection == NULL) {
        connection = create_connection(&parsed_url, quic_engine_ref->program_args.is_ipv4, quic_engine_ref);
        if (connection == NULL)
            return -1;
        try_destroy_parsed_url(&parsed_url, &is_text_destroyed);

        open_new_connection(quic_engine_ref, &parsed_url, connection);
        new_stream_ref->start_time = connection->start_time;
        lsquic_engine_process_conns(quic_engine_ref->engine_ref);
    } else {
        // time of the stream is measured with the start of connection or with the start of the stream
        // other time was excluded due to time clearance
        clock_gettime(CLOCK_MONOTONIC_RAW, &new_stream_ref->start_time);
    }
    try_destroy_parsed_url(&parsed_url, &is_text_destroyed);
    new_stream_ref->connection_params_ref = (void*)connection;
    copy_char_sequence(connection->ip->text, connection->ip->size, &new_stream_ref->data_pointer->used_ip,
                       &new_stream_ref->data_pointer->used_ip_str_len);
    insert_list_element(connection->stream_list, 0, (void*)new_stream_ref);

    quic_engine_ref->reading_streams_count++;
    lsquic_conn_make_stream(connection->connection_ref);

    return 0;
}

// more complex context is destroyed on close events
void destroy_stream(stream_parameters_t* stream_ref)
{
    lsquic_stream_close(stream_ref->stream_ref);
}

void destroy_connections(quic_engine_parameters* quic_engine_ref)
{
    struct enumerator* conn_enumerator = get_enumerator(quic_engine_ref->connections);
    for(connection_parameters* connection = (connection_parameters*)get_next_element(conn_enumerator);
            connection != NULL; connection = (connection_parameters*)get_next_element(conn_enumerator))
    {
        struct enumerator* stream_enumerator = get_enumerator(connection->stream_list);
        for(stream_parameters_t* stream = (stream_parameters_t*)get_next_element(stream_enumerator);
                stream != NULL; stream = (stream_parameters_t*)get_next_element(stream_enumerator))
        {
            destroy_stream(stream);
        }

        lsquic_conn_close(connection->connection_ref);
        free(stream_enumerator);
    }
    free(conn_enumerator);
}


