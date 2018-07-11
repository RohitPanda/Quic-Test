/*
 * connection_init_handler.h
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */

#ifndef SRC_CONNECTION_INIT_HANDLER_H_
#define SRC_CONNECTION_INIT_HANDLER_H_

#include <lsquic.h>

void http_client_on_stream_close(lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h);
void http_client_on_conn_closed (lsquic_conn_t *conn);

lsquic_stream_ctx_t * http_client_on_new_stream (void *stream_if_ctx, lsquic_stream_t *stream);
lsquic_conn_ctx_t * http_client_on_new_conn (void *stream_if_ctx, lsquic_conn_t *conn);
void http_client_on_connection_handshake(lsquic_conn_t* conn, int ok);

#endif /* SRC_CONNECTION_INIT_HANDLER_H_ */
