/*
 * connection_write_handler.h
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */

#ifndef SRC_CONNECTION_WRITE_HANDLER_H_
#define SRC_CONNECTION_WRITE_HANDLER_H_
#include <lsquic.h>

void http_client_on_write (lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h);

#endif /* SRC_CONNECTION_WRITE_HANDLER_H_ */
