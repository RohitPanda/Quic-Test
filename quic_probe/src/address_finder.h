/*
 * address_finder.h
 *
 *  Created on: Apr 13, 2018
 *      Author: sergei
 */

#ifndef SRC_ADDRESS_FINDER_H_
#define SRC_ADDRESS_FINDER_H_

#include <stddef.h>
#include <sys/socket.h>
#include "connection_params.h"
#include "error_report.h"

int parse_url(char *url, parsed_url_t* parsed_url_out_ref, error_report_t** error_ref_out);
int get_sockaddr(const char* host_name, struct sockaddr_storage * address_ref_out, int is_ipv4,
        error_report_t** error_ref_out);
text_t* sockaddr_to_ip_string(struct sockaddr* socket_address);
text_t* sockaddrstorage_to_ip_string(struct sockaddr_storage *socket_address);

#endif /* SRC_ADDRESS_FINDER_H_ */
