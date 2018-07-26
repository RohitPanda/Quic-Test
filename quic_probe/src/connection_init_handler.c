/*
 * connection_init_handler.c
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */

#include "connection_init_handler.h"


#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#include "engine_structs.h"
#include "timedifference.h"
#include "socket_handler.h"

extern int is_debug;

void http_client_on_stream_close(lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h)
{
    if (st_h == NULL)
        return;
    stream_parameters_t* stream_params = (stream_parameters_t*)st_h;
    // test
    if (!stream_params->is_read)
    {
        stream_params->is_read = 1;
        report_stream_error(init_text_from_const("stream was closed before getting result"), 0, stream_params,
                            init_text_from_const(__FILE__), __LINE__, QUIC_CLIENT_CODE_CONNECTION_FAIL);
        quic_engine_parameters* engine = extract_stream_engine(stream_params);
        engine->reading_streams_count--;
        if (engine->reading_streams_count == 0)
        {
            event_base_loopbreak(engine->events.event_base_ref);
        }
    }
    connection_parameters* conn_parameters = stream_params->connection_params_ref;
    if (stream_params->data_pointer != NULL)
        destroy_output_data_container(stream_params->data_pointer);
    remove_list_element_by_data(conn_parameters->stream_list, (void*)stream_params);
    destroy_text(stream_params->path);
    free(stream_params);
    stream_params->is_closed = 1;
}

void http_client_on_conn_closed (lsquic_conn_t *conn)
{
    connection_parameters* connection = (connection_parameters*)lsquic_conn_get_ctx(conn);
    // free port
    quic_engine_parameters* engine = (quic_engine_parameters*)connection->quic_engine_params_ref;

    close_socket(connection->socket_data.socket_id);
    event_free(connection->socket_data.read_event);
    destroy_socket_buffer(&connection->socket_data.socket_buffer);
    if (connection->socket_data.port_number != NULL)
        queue_put_data(engine->free_ports_queue, (void*)connection->socket_data.port_number);

    destroy_text(connection->hostname);
    destroy_text(connection->ip);
    destroy_simple_list(connection->stream_list);

    remove_list_element_by_data(engine->connections, connection);
    free(connection);
}

stream_parameters_t* find_not_created_stream_parameters(connection_parameters* connection)
{
    struct enumerator* enum_conn = get_enumerator(connection->stream_list);
    stream_parameters_t* stream_params = (stream_parameters_t*)get_next_element(enum_conn);
    while (stream_params != NULL)
    {
        if (!stream_params->is_created)
            break;
        stream_params = (stream_parameters_t*)get_next_element(enum_conn);
    }
    free(enum_conn);
    return stream_params;
}

lsquic_stream_ctx_t * http_client_on_new_stream (void* stream_if_ctx, lsquic_stream_t *stream)
{

    lsquic_conn_t* stream_connection = lsquic_stream_conn(stream);
    connection_parameters* conn_params = (connection_parameters*)lsquic_conn_get_ctx(stream_connection);
    if (conn_params == NULL)
        return NULL;

    stream_parameters_t* free_stream = find_not_created_stream_parameters(conn_params);
    free_stream->is_created = 1;
    free_stream->is_header_sent = 0;
    free_stream->is_closed = 0;
    free_stream->used_temp_buffer = 0;
    free_stream->header_info_ref = NULL;
    free_stream->is_read = 0;
    free_stream->stream_ref = stream;
    lsquic_stream_wantwrite(stream, 1);
    return (void*)free_stream;

}

lsquic_conn_ctx_t * http_client_on_new_conn (void* conn_if_ctx, lsquic_conn_t* conn)
{
	return lsquic_conn_get_ctx(conn);
}

void http_client_on_connection_handshake(lsquic_conn_t* conn, int ok)
{
    connection_parameters* conn_params = (connection_parameters*)lsquic_conn_get_ctx(conn);
    if (is_debug)
    {
        printf("handshake completed:%s\n", conn_params->hostname->text);
    }

    if (ok)
    {
        struct timespec now_time;
        clock_gettime(CLOCK_MONOTONIC_RAW, &now_time);

        if (conn_params->handshake_time.tv_sec == 0 && conn_params->handshake_time.tv_nsec == 0)
        {
            fill_time_difference_with_now(&conn_params->start_time, &conn_params->handshake_time);
        }
    } else {
        quic_engine_parameters* engine = (quic_engine_parameters*) conn_params->quic_engine_params_ref;
        report_engine_error(init_text_from_const("connection handshake failed"), 0, engine,
                            init_text_from_const(__FILE__), __LINE__, QUIC_CLIENT_CODE_CONNECTION_FAIL);
        lsquic_conn_close(conn);
    }
}