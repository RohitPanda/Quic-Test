//
// Created by sergei on 28.05.18.
//

#ifndef QUIC_PROBE_TEXT_H
#define QUIC_PROBE_TEXT_H

#include <stddef.h>

typedef struct text{
    char* text;
    size_t size;
} text_t;

void copy_char_sequence(const char* source, size_t source_size, char** dest_ref, size_t* dest_size_ref);
text_t* init_text(const char* chars, size_t size);
text_t* init_text_from_const(const char* chars);
text_t* copy_text(text_t* source);
void destroy_text(text_t* text);

#endif //QUIC_PROBE_TEXT_H
