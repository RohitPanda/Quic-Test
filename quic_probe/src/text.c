//
// Created by sergei on 28.05.18.
//

#include <stdlib.h>
#include <string.h>
#include "text.h"

void copy_char_sequence(const char* source, size_t source_size, char** dest_ref, size_t* dest_size_ref) {
    *dest_size_ref = source_size;
    *dest_ref = (char*)malloc(source_size + 1);
    memcpy(*dest_ref, source, source_size);
    (*(dest_ref))[source_size] = '\0';
}

text_t* init_text(const char* chars, size_t size){
    text_t* text = (text_t*)malloc(sizeof(text_t));
    copy_char_sequence(chars, size, &text->text, &text->size);
    return text;
}

text_t* copy_text(text_t* source) {
    return init_text(source->text, source->size);
}

text_t* init_text_from_const(const char* chars){
    return init_text(chars, strlen(chars));
}

void destroy_text(text_t* text){
    free(text->text);
    free(text);
}

