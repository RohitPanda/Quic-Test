//
// Created by sergei on 14.05.18.
//

#ifndef QUIC_PROBE_OUTPUT_DATA_H
#define QUIC_PROBE_OUTPUT_DATA_H

#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>

typedef struct quic_args {
    int is_ipv4;

    // Q035, Q039 or Q043
    const char* quic_ver_str;
    size_t quic_ver_str_size;

    // can be 0
    size_t timeout_ms;
    // for now lsquic does not support previous connection restoration
    int use_prev_conn_data;
    // preferred port numbers or NULL
    uint* local_port_numbers;
    // port counts of previous field or 0
    unsigned short port_count;
    // maximum streams for lsquic, better to have space ones for redirecting
    unsigned char max_streams;
    // write debug messages
    int is_debug;
    // use multiple streams for connection for the same host
    int is_one_conn_per_stream;
} quic_args;

struct url {
    // url text
    char* url_data;
    size_t url_len;
};

struct download_buffer {
    // allocated buffer, must be greater then downloaded data or null if header_only
    char* buffer;
    // buffer size
    size_t allocated_size;
    // return value of data size
    size_t used_size;
};

struct download_request{
    // url source
    struct url url_request;
    // allocated buffer, should be done in advance
    struct download_buffer buffer;
    // function to write data by chunks as soon as it arrive (buffer, size, arguments) or NULL
    size_t (*on_write)(void*, size_t, void*);
    // arguments for on_write
    void* write_argument;
    // maximum chunk size to download
    size_t max_write_size;
    // is header downloading only required, can be used for getting size of source with output_data.data_size
    int header_only;
};

#define QUIC_CLIENT_CODE_OK 0
#define QUIC_CLIENT_CODE_TIMEOUT 101
#define QUIC_CLIENT_CODE_CONNECTION_FAIL 102
#define QUIC_CLIENT_CODE_SOCKET_FAIL 103
#define QUIC_CLIENT_CODE_BUFFER_OVERFLOW 104
#define QUIC_CLIENT_CODE_BAD_RESPONSE 105


struct output_data {
    // error code or 0 on success
    int error_code;
    // error text
    char* error_str;
    size_t error_size;
    // ip of used resource
    char* used_ip;
    size_t used_ip_str_len;
    // initial request
    struct download_request* request_args;
    // final url as page can be redirected
    struct url final_url;
    // http header version, for example, HTTP/1.1
    char http_header[10];
    int http_code;
    // time to establish a connection for lsquic
    struct timespec connection_time;
    // time to download a resource
    struct timespec download_time;
    // reports content-length header value if only header was downloaded or real used buffer size
    size_t data_size;
};
// use every time after receiving a result
void destroy_output_data_container(struct output_data* data_ref);
struct output_data* init_output_data_container(char* url, size_t url_size);
struct output_data* init_empty_output_data_container();

#endif //QUIC_PROBE_OUTPUT_DATA_H
