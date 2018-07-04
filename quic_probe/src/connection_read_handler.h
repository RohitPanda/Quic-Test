/*
 * connection_read_handler.h
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */

#ifndef SRC_CONNECTION_READ_HANDLER_H_
#define SRC_CONNECTION_READ_HANDLER_H_

#include <lsquic.h>

void http_client_on_read (lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h);
int on_packets_out(void *packets_out_ctx, const struct lsquic_out_spec  *out_spec, unsigned n_packets_out);

#endif /* SRC_CONNECTION_READ_HANDLER_H_ */
