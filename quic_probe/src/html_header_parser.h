//
// Created by sergei on 02.06.18.
//

#ifndef QUIC_PROBE_HTML_HEADER_PARSER_H
#define QUIC_PROBE_HTML_HEADER_PARSER_H

#include <sys/types.h>
#include <stddef.h>

#include "error_report.h"
#include "text.h"

struct header_info{
    uint response_code;
    text_t* redirect_url;
    text_t* http_version;
    size_t content_length;
};

struct header_info* get_response_header(char* data_pointer, size_t size, error_report_t** error_out,
        int is_content_len_required);
void free_http_header(struct header_info **header_ref);
#endif //QUIC_PROBE_HTML_HEADER_PARSER_H
