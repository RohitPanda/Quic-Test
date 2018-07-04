//
// Created by sergei on 24.05.18.
//

#include "../include/args_data.h"
#include "text.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

struct output_data* init_empty_output_data_container()
{
    struct output_data* container = malloc(sizeof(struct output_data));
    container->error_code = QUIC_CLIENT_CODE_OK;
    container->error_str = NULL;
    container->used_ip = NULL;
    container->used_ip_str_len = 0;
    container->final_url.url_data = NULL;
    container->final_url.url_len = 0;
    container->http_code = 0;
    return container;
}


struct output_data* init_output_data_container(char* url, size_t url_size)
{
    struct output_data* container = init_empty_output_data_container();
    copy_char_sequence(url, url_size, &container->final_url.url_data, &container->final_url.url_len);
    return container;
}

void destroy_output_data_container(struct output_data* data_ref)
{
    if (data_ref->error_str != NULL)
        free(data_ref->error_str);
    if (data_ref->final_url.url_data != NULL)
        free(data_ref->final_url.url_data);
    if (data_ref->used_ip != NULL)
        free(data_ref->used_ip);
    free(data_ref);
}

