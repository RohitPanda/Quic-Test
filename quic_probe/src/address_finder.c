/*
 * address_finder.c
 *
 *  Created on: Apr 13, 2018
 *      Author: sergei
 */

#include "address_finder.h"

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <sys/types.h>

#define MAXMATCH 2
#define QUIC_PORT 443
#define ERROR_MESSAGE_SIZE 1000

void set_quic_port(struct sockaddr_storage* addr_ref, int is_ipv4)
{
	if (is_ipv4)
		((struct sockaddr_in*) addr_ref)->sin_port = htons(QUIC_PORT);
	else
		((struct sockaddr_in6*) addr_ref)->sin6_port = htons(QUIC_PORT);
}

int get_sockaddr(const char* host_name, struct sockaddr_storage* address_ref_out, int is_ipv4,
		error_report_t** error_ref_out)
{
	int required_family = is_ipv4? AF_INET : AF_INET6;
	struct addrinfo* addresses;
	struct addrinfo hint;
	hint.ai_family = required_family;
	hint.ai_socktype = SOCK_DGRAM;
	hint.ai_flags = AI_ALL;
	hint.ai_protocol = 0;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	int get_addr_info_status = getaddrinfo(host_name, NULL, &hint, &addresses);
	if (get_addr_info_status != 0)
	{
		char error_text[ERROR_MESSAGE_SIZE];
		int text_size = sprintf(error_text, "For host %s ipv%d address was not found", host_name, is_ipv4? 4 : 6);
		*error_ref_out = create_report(init_text(error_text, text_size), 0, __FILE__, __LINE__,
				QUIC_CLIENT_CODE_SOCKET_FAIL);
		return -1;
	}
	struct addrinfo* addr_i_ref = addresses;
	memcpy(address_ref_out, addr_i_ref->ai_addr, addr_i_ref->ai_addrlen);
	set_quic_port(address_ref_out, is_ipv4);
	freeaddrinfo(addresses);
	return 0;
}

int parse_url(char * url, parsed_url_t* parsing_out_ref, error_report_t** error_ref_out)
{
    regex_t regex;
    regmatch_t matches[MAXMATCH];
    int status;
    // extract http://www.google.com/some_other_url_part_which_does_not_matter
    status = regcomp(&regex, "https?://([^/]+)/?", REG_EXTENDED);
    status |= regexec(&regex, url, MAXMATCH, matches, 0);
    if (status)
    {
    	*error_ref_out = create_report(init_text_from_const("Host not found for url"), 0, __FILE__, __LINE__,
    			QUIC_CLIENT_CODE_SOCKET_FAIL);
        return -1;
    }
    // first match is the general match and not a specific one for the grouped symbols ()
    parsing_out_ref->hostname = init_text(url + matches[1].rm_so, (size_t)(matches[1].rm_eo - matches[1].rm_so));
    parsing_out_ref->path = init_text(url + matches[1].rm_eo, (size_t)(strlen(url) - matches[1].rm_eo));

    regfree(&regex);
    return 0;
}

text_t* sockaddr_to_ip_string(struct sockaddr* socket_address)
{
	void* addr;

	if (socket_address->sa_family == AF_INET)
	{
		struct sockaddr_in* sock_addr4 = (struct sockaddr_in*) socket_address;
		addr = &sock_addr4->sin_addr;
	} else if (socket_address->sa_family == AF_INET6)
	{
		struct sockaddr_in6* sock_addr6 = (struct sockaddr_in6*) socket_address;
		addr = &sock_addr6->sin6_addr;
	}
	char buf[INET6_ADDRSTRLEN];
	inet_ntop(socket_address->sa_family, addr, buf, INET6_ADDRSTRLEN);
	return init_text(buf, socket_address->sa_family == AF_INET? INET_ADDRSTRLEN: INET6_ADDRSTRLEN);
}

text_t* sockaddrstorage_to_ip_string(struct sockaddr_storage *socket_address)
{
	return sockaddr_to_ip_string((struct sockaddr*)socket_address);
}
