//
// Created by sergei on 08.05.18.
//

#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "socket_buffer.h"


void init_socket_buffer(struct socket_data *socket_buffer_ref, int socket_id, int max_packet_size,
                        socklen_t socket_buffer_size)
{
    // socket buffer in fact can contain 2 times more data
    int max_quic_packets =  socket_buffer_size / max_packet_size * 2;

    // init buffers with max sizes
    socket_buffer_ref->data_block = malloc(socket_buffer_size);
    socket_buffer_ref->data_size = 0;

    socket_buffer_ref->messages = calloc(max_quic_packets, sizeof(struct buffered_socket_message));
    socket_buffer_ref->messages_len = 0;

}
void destroy_socket_buffer(struct socket_data *socket_buffer_ref)
{
    free(socket_buffer_ref->messages);
    free(socket_buffer_ref->data_block);


}