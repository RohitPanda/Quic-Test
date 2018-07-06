//
// Created by sergei on 08.05.18.
//

#ifndef QUIC_PROBE_SOCKET_BUFFER_H
#define QUIC_PROBE_SOCKET_BUFFER_H

#include <stddef.h>
#include <sys/uio.h>
#include <bits/socket.h>

struct buffered_socket_message
{
    struct iovec message;
    struct sockaddr_storage peer_addr;
    struct sockaddr_storage local_addr;

};

struct socket_data
{
    unsigned char* data_block;
    size_t data_size;

    struct buffered_socket_message* messages;
    unsigned int messages_len;
};

void init_socket_buffer(struct socket_data *socket_buffer_ref, int socket_id, int max_packet_size,
                        socklen_t socket_buffer_size);
void destroy_socket_buffer(struct socket_data *socket_buffer_ref);

#endif //QUIC_PROBE_SOCKET_BUFFER_H
