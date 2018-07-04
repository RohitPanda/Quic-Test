//
// Created by sergei on 14.05.18.
//

#ifndef QUIC_PROBE_MEMORY_BUFFER_H
#define QUIC_PROBE_MEMORY_BUFFER_H

#include <stdlib.h>

// cycle buffer, so no need to restart a buffer on full fill. In the end it restarts from the beginning

struct cycle_buffer{
    size_t max_size;
    size_t partition_size;
    size_t current_pointer;
    char* buffer;
};

void init_memory_buffer(struct cycle_buffer *buffer_ref, size_t max_size);
void reset_memory_buffer_allocations(struct cycle_buffer *buffer_ref, int streams_count);
void destroy_memory_buffer(struct cycle_buffer *buffer_ref);
char* get_memory_pointer(struct cycle_buffer* buffer_ref);

#endif //QUIC_PROBE_MEMORY_BUFFER_H
