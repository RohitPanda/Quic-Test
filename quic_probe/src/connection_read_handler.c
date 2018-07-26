/*
 * connection_read_handler.c
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */

#include "connection_read_handler.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/param.h>

#include "timedifference.h"
#include "engine_structs.h"
#include "html_header_parser.h"
#include "connection_establisher.h"
#include "address_finder.h"

#define ERROR_MESSAGE_SIZE 200

extern int is_debug;

#define END_HEADER_SIZE 4
const char END_HEADER[END_HEADER_SIZE + 1] = "\r\n\r\n";


int check_end_sequence(char* data, size_t length, size_t* end_position)
{
	size_t pos = 0;
	uint current_header_sequence = 0;

	while(pos < length)
	{
		if (data[pos] == END_HEADER[current_header_sequence])
		{
			current_header_sequence++;
			if (current_header_sequence == END_HEADER_SIZE)
			{
				*end_position = pos + 1;
				return 1;
			}
		} else if (current_header_sequence != 0)
		{
			pos-= current_header_sequence;
			current_header_sequence = 0;
		}
		pos++;
	}
	return 0;
}

size_t on_read_to_buffer(stream_parameters_t* stream, size_t added_size)
{
	stream->data_pointer->request_args->buffer.used_size += added_size;
	if (stream->data_ref == NULL) {
		if (stream->data_pointer->request_args->on_write != NULL)
			stream->data_pointer->request_args->on_write((void*)stream->temp_buffer, added_size,
														 stream->data_pointer->request_args->write_argument);
		return added_size;
	}
	// call request to notify on download_request
	if (stream->data_pointer->request_args->on_write != NULL)
		stream->data_pointer->request_args->on_write((void*)stream->data_ref, added_size,
													 stream->data_pointer->request_args->write_argument);
	stream->buffer_left -= added_size;
	stream->data_ref += added_size;
	if (stream->buffer_left <= 0)
	{
		report_stream_error(init_text_from_const("Buffer output overflow"), errno, stream,
							init_text_from_const(__FILE__), __LINE__, QUIC_CLIENT_CODE_BUFFER_OVERFLOW);
		lsquic_stream_close(stream->stream_ref);
		return -1;
	}
	return added_size;
}

size_t copy_header_remainder_to_buffer(stream_parameters_t* stream, size_t data_start_position)
{
	size_t added_size = stream->used_temp_buffer - data_start_position;
	if (added_size == 0)
		return 0;
	if (stream->data_ref != NULL) {
		memcpy(stream->data_ref, stream->temp_buffer + data_start_position, added_size);
	} else {
		memmove(stream->temp_buffer, stream->temp_buffer + data_start_position, added_size);
	}

	return on_read_to_buffer(stream, added_size);
}

size_t read_header(stream_parameters_t* stream)
{
    // check if buffer was filled and it's required to fill it more with header
    if (stream->used_temp_buffer >= MAX_HTTP_HEADER_SIZE)
    {
        report_stream_error(init_text_from_const("Buffer output overflow"), errno, stream,
                            init_text_from_const(__FILE__), __LINE__, QUIC_CLIENT_CODE_BUFFER_OVERFLOW);
        lsquic_stream_close(stream->stream_ref);
        return -1;
    }

	size_t prev_header_size = stream->used_temp_buffer;
	uint possible_offset = MIN(END_HEADER_SIZE, prev_header_size);
	ssize_t read_len = lsquic_stream_read(stream->stream_ref, (void*) (stream->temp_buffer + prev_header_size),
	        MIN(MAX_HTTP_HEADER_SIZE - stream->used_temp_buffer, stream->data_pointer->request_args->max_write_size));
	if (read_len <= 0)
	    return read_len;

	stream->used_temp_buffer += read_len;

	size_t header_len;
	int is_header_ended = check_end_sequence(stream->temp_buffer + prev_header_size - possible_offset,
			read_len + possible_offset, &header_len) || (read_len == 0);
	if (is_header_ended)
	{
		error_report_t* error_report = NULL;
		stream->header_info_ref = get_response_header(stream->temp_buffer, header_len, &error_report,
				stream->data_pointer->request_args->header_only);
		if (error_report != NULL)
		{
			report_stream_error(error_report->error, error_report->error_code, stream, error_report->filename,
								error_report->line, error_report->local_error_code);
			destroy_report(error_report);
			return -1;
		}
		stream->data_pointer->http_code = stream->header_info_ref->response_code;

		if (!stream->data_pointer->request_args->header_only)
		    copy_header_remainder_to_buffer(stream, prev_header_size - possible_offset + header_len);
	}
	return read_len;
}

size_t read_data(stream_parameters_t *stream)
{
	// read until header is formed, rest data feed
	if (stream->header_info_ref == NULL) {
		return read_header(stream);
	}

	ssize_t nread;
	if (stream->data_ref == NULL) {
		nread = lsquic_stream_read(stream->stream_ref, (void*)stream->temp_buffer, MAX_HTTP_HEADER_SIZE);
	} else {
		nread = lsquic_stream_read(stream->stream_ref, (void*)stream->data_ref,
				MIN(stream->buffer_left, stream->data_pointer->request_args->max_write_size));
	}

	if (nread <= 0)
		return nread;
	return on_read_to_buffer(stream, nread);

}

void fix_time(stream_parameters_t* stream_parameters)
{
	struct timespec now_time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &now_time);

	connection_parameters* connection = (connection_parameters*) stream_parameters->connection_params_ref;
	fill_time_difference_with_now(&stream_parameters->start_time, &stream_parameters->data_pointer->download_time);

	stream_parameters->data_pointer->connection_time = connection->handshake_time;
}

void post_ready_data(stream_parameters_t* stream_parameters, quic_engine_parameters* engine)
{
	put_stack_element(engine->output_stack_ref, stream_parameters->data_pointer);
	// remove output container out of scope of stream destructor (on_stream_close)
	stream_parameters->data_pointer = NULL;
	// if there are no streams, then all stream results were posted
	if (engine->reading_streams_count == 0)	{
		event_base_loopbreak(engine->events.event_base_ref);
	}

}

void report_http_code_error(stream_parameters_t *stream_parameters, struct header_info *http_result)
{
	char error[ERROR_MESSAGE_SIZE];
	int actual_error_size = sprintf(error, "http response code %d", http_result->response_code);
	report_stream_error(init_text(error, actual_error_size), 0, stream_parameters, init_text_from_const(__FILE__),
						__LINE__, QUIC_CLIENT_CODE_BAD_RESPONSE);
	if (http_result->redirect_url != NULL)
		free(http_result->redirect_url);
}

void process_finished_stream(stream_parameters_t* stream_parameters)
{
	connection_parameters* conn = stream_parameters->connection_params_ref;
	quic_engine_parameters* engine = conn->quic_engine_params_ref;

	// fix the time, even if the code is error or there is redirect code
	fix_time(stream_parameters);

	engine->reading_streams_count--;

	struct header_info* header_info = stream_parameters->header_info_ref;
	strncpy(stream_parameters->data_pointer->http_header, header_info->http_version->text,
            header_info->http_version->size);
	stream_parameters->data_pointer->data_size = stream_parameters->data_pointer->request_args->header_only?
			header_info->content_length : stream_parameters->data_pointer->request_args->buffer.used_size;
	// success 2xx codes
	if (header_info->response_code >= 200 && header_info->response_code < 300)
	{
	    stream_parameters->data_pointer->error_code = QUIC_CLIENT_CODE_OK;
		post_ready_data(stream_parameters, engine);

	} else if (header_info->response_code >= 300 && header_info->response_code < 400) // 3xx redirect codes
	{
	    charge_request(engine, stream_parameters->data_pointer->request_args, header_info->redirect_url,
                       stream_parameters->start_time);
		stream_parameters->is_closed = 1;
		destroy_output_data_container(stream_parameters->data_pointer);
		stream_parameters->data_pointer = NULL;
	} else
	{
		// else there is an error
		report_http_code_error(stream_parameters, header_info);
		if (engine->reading_streams_count == 0)	{
			event_base_loopbreak(engine->events.event_base_ref);
		}
	}
	// destroyed by charge_request function in case of redirect
	free_http_header(&header_info);
}

void http_client_on_read (lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h)
{
	stream_parameters_t* stream_parameters = (stream_parameters_t*) st_h;
	if (stream_parameters->data_pointer == NULL)
	    return;
	if (is_debug)
	{
		printf("stream on read:%s\n", stream_parameters->data_pointer->final_url.url_data);
	}

    ssize_t read_len;
	int has_end_code = 0;
	do
	{
		read_len = read_data(stream_parameters);
		if (stream_parameters->header_info_ref != NULL)
		{
			uint response_code = stream_parameters->header_info_ref->response_code;
			if ((response_code != 200) && (response_code != 0))
				has_end_code = 1;
		}

	} while (read_len > 0 && !has_end_code);

	if (read_len < 0)
	{
	    if (errno == EWOULDBLOCK) {
            return;
        }
	    else
        {
            report_stream_error(init_text_from_const("read error"), errno, stream_parameters,
                                init_text_from_const(__FILE__), __LINE__, QUIC_CLIENT_CODE_CONNECTION_FAIL);
            stream_parameters->is_read = 1;
            lsquic_stream_close(stream);
            return;
        }
	}
	stream_parameters->is_read = 1;
    lsquic_stream_close(stream);
    process_finished_stream(stream_parameters);
}

int on_packets_out(void *packets_out_ctx, const struct lsquic_out_spec *out_spec, unsigned n_packets_out)
{
	struct iovec iov;
	quic_engine_parameters* engine_params_ref = (quic_engine_parameters*)packets_out_ctx;
	struct msghdr message_header;

	message_header.msg_iov = &iov;

	struct sockaddr_storage destination_addr;
	for(unsigned int i = 0; i < n_packets_out; i++)
	{
	    size_t address_size = out_spec[i].dest_sa->sa_family == AF_INET?
	            sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);

	    memcpy(&destination_addr, out_spec[i].dest_sa, address_size);
		connection_parameters* connection = (connection_parameters*)out_spec[i].peer_ctx;
		if (is_debug)
        {
            text_t* ip_str = sockaddrstorage_to_ip_string(&destination_addr);
            printf("out packets to ip: %s\n", ip_str->text);
            destroy_text(ip_str);
        }
		iov.iov_base = (void *) out_spec[i].buf;
		iov.iov_len = out_spec[i].sz;
		message_header.msg_name       = (void*)&destination_addr;
		message_header.msg_namelen    = out_spec[i].dest_sa->sa_family == AF_INET?
										sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
		message_header.msg_iov        = &iov;
		message_header.msg_iovlen     = 1;
		message_header.msg_flags      = 0;
		message_header.msg_control 	  = NULL;
		message_header.msg_controllen = 0;

		ssize_t send_status = sendmsg(connection->socket_data.socket_id, &message_header, 0);
		// packet sending was failed
		if (send_status < 0)
		{
			report_engine_error(init_text_from_const("cannot send packets out"), errno, engine_params_ref,
								init_text_from_const(__FILE__), __LINE__, QUIC_CLIENT_CODE_SOCKET_FAIL);
		}
	}
	return (int)n_packets_out;
}
