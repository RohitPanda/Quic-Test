//
// Created by sergei on 09.05.18.
//

#ifndef QUIC_PROBE_QUIC_DOWNLOADER_H
#define QUIC_PROBE_QUIC_DOWNLOADER_H

#include <stdio.h>
#include "args_data.h"

struct quic_engine_shell;
/*
 * creates a quic client, that is able to download requested resources, build upon lsquic
 * allocates memory into engine_parameters_ref with selected arguments
 * returns -1 on failure
 */
int create_client(struct quic_engine_shell** engine_parameters_ref, quic_args* quic_args_ref);
// don't forget to destroy a client after having job done
void destroy_client(struct quic_engine_shell* engine_ref_out);
// put request in a queue
int start_downloading(struct quic_engine_shell* engine_ref, struct download_request* requests, int request_count);

// starts downloading requested links and returns first processed result. uses current thread as processing power.
// if request queue is empty returns null
// don`t forget to free the memory of output_data with destroy_output_data_container as it's not static
struct output_data* get_download_result(struct quic_engine_shell* engine_ref);

#endif //QUIC_PROBE_QUIC_DOWNLOADER_H
