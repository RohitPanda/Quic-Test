//
// Created by sergei on 14.05.18.
//

#include "cycle_buffer.h"


void init_memory_buffer(struct cycle_buffer *buffer_ref, size_t max_size)
{
    buffer_ref->max_size = max_size;
    buffer_ref->current_pointer = 0;
    buffer_ref->buffer = malloc(max_size);
}

void reset_memory_buffer_allocations(struct cycle_buffer *buffer_ref, int streams_count)
{
    buffer_ref->current_pointer = 0;
    buffer_ref->partition_size = buffer_ref->max_size / streams_count;
}

void destroy_memory_buffer(struct cycle_buffer *buffer_ref)
{
    free(buffer_ref->buffer);
}

char* get_memory_pointer(struct cycle_buffer* buffer_ref)
{
    if (buffer_ref->current_pointer > buffer_ref->max_size)
        buffer_ref->current_pointer = 0;
    char* pointer = buffer_ref->buffer + buffer_ref->current_pointer;
    buffer_ref->current_pointer += buffer_ref->partition_size;
    return pointer;
}