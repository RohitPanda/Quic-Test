//
// Created by sergei on 07.05.18.
//

#ifndef QUIC_PROBE_CONNECTION_PARAMS_H
#define QUIC_PROBE_CONNECTION_PARAMS_H

#include <lsquic_types.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "simple_list.h"
#include "socket_buffer.h"
#include "args_data.h"
#include "text.h"
#include "html_header_parser.h"

// 16 Kb, typically limited by servers with 8 for most, 16 for IIS, can be 48 for Tomcat but really unlucky
#define MAX_HTTP_HEADER_SIZE 16*1024

typedef struct parsed_url {
    text_t* hostname;
    text_t* path;
} parsed_url_t;


struct socket_shell {
    struct event* read_event;
    struct socket_data socket_buffer;
    int socket_id;
    socklen_t socket_buffer_size;
    uint* port_number;
};

typedef struct stream_parameters {
    int is_header_sent;
    text_t* path;
    lsquic_stream_t *stream_ref;
    void* connection_params_ref;

    int is_created;
    struct output_data* data_pointer;
    size_t buffer_left;
    char* data_ref;
    struct timespec start_time;
    int is_closed;
    int is_read;

    char temp_buffer[MAX_HTTP_HEADER_SIZE];
    size_t used_temp_buffer;
    struct header_info* header_info_ref;

} stream_parameters_t;

typedef struct connection_params {
    struct socket_shell socket_data;
    struct sockaddr_storage destination_addr;
    struct sockaddr_storage local_addr;
    struct simple_list* stream_list;
    lsquic_conn_t* connection_ref;
    text_t* hostname;
    struct timespec start_time;
    struct timespec handshake_time;
    text_t* ip;
    void* quic_engine_params_ref;
} connection_parameters;

connection_parameters* find_connection(struct simple_list* connections, const char* hostname, int is_dedicated_stream);

#endif //QUIC_PROBE_CONNECTION_PARAMS_H
