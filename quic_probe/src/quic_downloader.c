//
// Created by sergei on 09.05.18.
//

#include <pthread.h>
#include <string.h>
#include "../include/quic_downloader.h"
#include "quic_engine_holder.h"
#include "connection_establisher.h"
#include "cycle_buffer.h"
#include "event_handlers.h"

/*
 * engine_ref_out - empty instance for memory allocation simplification
 * quic_args_ref - quic engine parameters for creation
 */
int create_client(quic_engine_parameters** engine_parameters_ref, quic_args* quic_args_ref)
{
    is_debug = quic_args_ref->is_debug;
    *engine_parameters_ref = malloc(sizeof(quic_engine_parameters));
    (*engine_parameters_ref)->program_args = *quic_args_ref;
    if (init_engine(*engine_parameters_ref) == -1){
        return -1;
    }
    return 0;
}
void destroy_client(quic_engine_parameters* engine_ref_out)
{
    destroy_engine(engine_ref_out);
    free(engine_ref_out);
}

int start_downloading(quic_engine_parameters* engine_ref, struct download_request* requests, int request_count)
{
    if (engine_ref->is_closed)
        return -1;
    for(int i = 0; i < request_count; i++)
    {
        text_t* url = init_text(requests[i].url_request.url_data, requests[i].url_request.url_len);
        struct timespec time_now;
        clock_gettime(CLOCK_MONOTONIC_RAW, &time_now);
        charge_request(engine_ref, &requests[i], url, time_now);
    }
    // process packets before and refresh engine to load with new request
    lsquic_engine_process_conns(engine_ref->engine_ref);
    schedule_engine(engine_ref);
    return 0;
}

struct output_data* get_download_result(quic_engine_parameters* engine_ref)
{
    if (engine_ref->is_closed)
        return NULL;
    void* result = pull_stack_element(engine_ref->output_stack_ref);
    if (result != NULL)
        return (struct output_data*)result;

    if ((engine_ref->reading_streams_count == 0) && (engine_ref->request_queue->length == 0))
        return result;

    event_base_loop(engine_ref->events.event_base_ref, 0);
    result = pull_stack_element(engine_ref->output_stack_ref);
    if (result == NULL)
        return NULL;
    return (struct output_data*) result;
}





