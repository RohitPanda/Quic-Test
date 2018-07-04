/*
 * engine_structs.c
 *
 *  Created on: Apr 18, 2018
 *      Author: sergei
 */

#include <string.h>
#include "engine_structs.h"
#include "../include/quic_downloader.h"
#include "text.h"

quic_engine_parameters* extract_stream_engine(stream_parameters_t *stream)
{
    connection_parameters* connection = (connection_parameters*)stream->connection_params_ref;
    quic_engine_parameters* engine_parameters = (quic_engine_parameters*) connection->quic_engine_params_ref;
    return engine_parameters;
}

#define ERROR_MESSAGE_BUFFER 1000

void fill_error(struct output_data* output, const char* custom_error, int error_code, const char* filename, int line,
                int local_error_code)
{
    char buffer[ERROR_MESSAGE_BUFFER];
    output->error_code = local_error_code;
    output->error_size = sprintf(buffer, "%s %s, %s:%d", custom_error, error_code == 0? "": strerror(error_code),
                                 filename, line);
    output->error_str = malloc((size_t)output->error_size);
    memcpy(output->error_str, buffer, (size_t)output->error_size);
}

void report_error(struct output_data* output, text_t* error_message, int error_code, quic_engine_parameters* engine,
                         text_t* file, int line, int local_error_code)
{
    fill_error(output, error_message->text, error_code, file->text, line, local_error_code);
    put_stack_element(engine->output_stack_ref, output);
    destroy_text(error_message);
    destroy_text(file);
}


void report_stream_error(text_t* error_message, int error_code, stream_parameters_t* stream, text_t* file, int line,
                         int local_error_code)
{
    quic_engine_parameters* engine = extract_stream_engine(stream);
    report_error(stream->data_pointer, error_message, error_code, engine, file, line, local_error_code);
    stream->data_pointer = NULL;
}

void report_engine_error(text_t* error_message, int error_code, quic_engine_parameters* engine, text_t* file, int line,
                         int local_error_code)
{
    struct output_data* output = init_empty_output_data_container();

    report_error(output, error_message, error_code, engine, file, line, local_error_code);
    engine->is_closed = 1;
    lsquic_global_cleanup();
    event_base_loopbreak(engine->events.event_base_ref);
}

void report_engine_error_with_report(quic_engine_parameters* engine, error_report_t* report)
{
    report_engine_error(report->error, report->error_code, engine, report->filename, report->line,
                        report->local_error_code);
    destroy_report(report);
}