/*
 * connection_establisher.h
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */

#ifndef SRC_CONNECTION_ESTABLISHER_H_
#define SRC_CONNECTION_ESTABLISHER_H_

#include "engine_structs.h"

#include <sys/time.h>



void charge_request(quic_engine_parameters* engine, struct download_request* request, struct text* final_url,
                    struct timespec start_time);

void connect_event_handler (int fd, short what, void *arg);

/*
 * add connection to the queue.
 * quic_engine_ref - reference to the shell of quic engine
 * request - original request with buffer and url
 * prev_data - additional data from previous request, can be null
 */
int quic_connect(quic_engine_parameters* quic_engine_ref, struct download_request* request, text_t* url);
void destroy_connections(quic_engine_parameters *quic_engine_ref);

#endif /* SRC_CONNECTION_ESTABLISHER_H_ */
