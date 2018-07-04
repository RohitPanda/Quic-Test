//
// Created by sergei on 16.05.18.
//


#include "engine_structs.h"
#include "lsquic_types.h"

#include "event_handlers.h"


void run_connection_streams_timeout(connection_parameters *connection_parameters, quic_engine_parameters* quic_engine)
{

    struct enumerator* list_enumerator = get_enumerator(connection_parameters->stream_list);
    int stream_count = get_list_size(connection_parameters->stream_list);
    for(stream_parameters_t* stream_parameters = get_next_element(list_enumerator); stream_parameters != NULL;
            stream_parameters = get_next_element(list_enumerator))
    {
        text_t* error = init_text_from_const("timeout");
        text_t* filename = init_text_from_const(__FILE__);
        report_stream_error(error, 0, stream_parameters, filename, __LINE__, QUIC_CLIENT_CODE_TIMEOUT);
        quic_engine->reading_streams_count--;
        stream_parameters->is_read = 1;
        lsquic_stream_close(stream_parameters->stream_ref);
    }
    if (stream_count > 0)
        lsquic_conn_close(connection_parameters->connection_ref);
    free(list_enumerator);
}

void run_timeout(quic_engine_parameters* quic_engine_ref)
{
    struct enumerator* list_enumerator = get_enumerator(quic_engine_ref->connections);
    for(connection_parameters* conn_parameters = get_next_element(list_enumerator); conn_parameters != NULL;
            conn_parameters = get_next_element(list_enumerator))
    {
        run_connection_streams_timeout(conn_parameters, quic_engine_ref);
        lsquic_conn_close(conn_parameters->connection_ref);
    }
    free(list_enumerator);
}

void schedule_engine(quic_engine_parameters* quic_engine_ref)
{
    struct timeval timeout;

    timeout.tv_sec = (unsigned) quic_engine_ref->program_args.timeout_ms / 1000;
    timeout.tv_usec = (unsigned) quic_engine_ref->program_args.timeout_ms % 1000;

    // timeout refreshes even if the previous one has not expired
    event_add(quic_engine_ref->events.event_timeout_ref, &timeout);
};

void on_timeout(quic_engine_parameters* quic_engine_ref){
    // no streams => no timeout
    if (quic_engine_ref->reading_streams_count == 0)
        return;

    run_timeout(quic_engine_ref);
}

void timer_handler (int fd, short what, void *arg)
{
    quic_engine_parameters* quic_engine_ref = (quic_engine_parameters*)arg;
    event_base_loopbreak(quic_engine_ref->events.event_base_ref);
    on_timeout(quic_engine_ref);
    lsquic_engine_process_conns(quic_engine_ref->engine_ref);
    if (get_stack_length(quic_engine_ref->output_stack_ref) == 0)
    {
        while(quic_engine_ref->reading_streams_count > 0)
        {
            report_engine_error(init_text_from_const("timeout without connections"), 0, quic_engine_ref,
                                init_text_from_const(__FILE__), __LINE__, QUIC_CLIENT_CODE_TIMEOUT);
            quic_engine_ref->reading_streams_count--;
        }
    }

    //schedule_engine(quic_engine_ref);
}

//void keep_alive(quic_engine_parameters* quic_engine_ref)
//{
//    lsquic_engine_process_conns(quic_engine_ref->engine_ref);
//    if (!quic_engine_ref->is_closed)
//    {
//        int usec_left;
//        if (lsquic_engine_earliest_adv_tick(quic_engine_ref, &usec_left))
//        {
//            struct timespec next_call = {
//                    .tv_sec = (unsigned) usec_left / 1000000,
//                    .tv_nsec = (unsigned) usec_left % 1000000
//            };
//            event_add(quic_engine_ref->events.event_keep_alive_ref, &next_call);
//        }
//    }
//}

//void keep_alive_handler (int fd, short what, void *arg)
//{
//    quic_engine_parameters* quic_engine_ref = (quic_engine_parameters*)arg;
//    keep_alive(quic_engine_ref);
//
//}
