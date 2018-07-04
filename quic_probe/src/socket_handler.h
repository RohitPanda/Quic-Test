/*
 * socket_hander.h
 *
 *  Created on: Apr 13, 2018
 *      Author: sergei
 */

#ifndef SRC_SOCKET_HANDLER_H_
#define SRC_SOCKET_HANDLER_H_

#include <sys/socket.h>
#include "engine_structs.h"
#include "error_report.h"

// opens a UPD socket and returns socket id
void read_socket_event(int fd, short flags, void *ctx);
void read_socket_packet(connection_parameters* connection);
int open_socket(int is_ipv4, struct sockaddr_storage* out_local_addr_ref, int* socket_id_out_ref,
                socklen_t* socket_buffer_size_out, int port_number, error_report_t** error_out);
void close_socket(int socket_id);

#endif /* SRC_SOCKET_HANDLER_H_ */
