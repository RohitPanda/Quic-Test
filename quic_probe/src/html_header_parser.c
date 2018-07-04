//
// Created by sergei on 02.06.18.
//

#include "html_header_parser.h"

#include <regex.h>
#include <stdlib.h>

#include "mono_parser.h"


// max size for header, usually HTTP/1.1 code code_text \n Redirect url: url(2048 practically), 500 for other headers
#define HEADER_HEAP 2548
#define HEADER_VERSION_MAX_SIZE 100
#define HTTP_UNTIL_CODE_SIZE 20
#define MIN(A,B) A<B?A:B

int get_code(char* data_pointer, size_t size, error_report_t** error_out)
{
    text_t* header = init_text(data_pointer, MIN(HTTP_UNTIL_CODE_SIZE, size));
    text_t* code_text = parse_first(header->text, "HTTP\\S+ ([0-9]+)", error_out);
    destroy_text(header);
    if (*error_out != NULL)
        return -1;
    int code = atoi(code_text->text);
    destroy_text(code_text);
    return code;
}

text_t* get_redirect_url(char* data_pointer, size_t size, error_report_t** error_out)
{
    text_t* header = init_text(data_pointer, MIN(HEADER_HEAP, size));
    text_t* url = parse_first(header->text, "location:\\s+(\\S+)\r", error_out);
    destroy_text(header);
    if (*error_out != NULL)
        return NULL;
    return url;
}

text_t* get_http_version(char* data_pointer, size_t size, error_report_t** error_out)
{
    text_t* header = init_text(data_pointer, MIN(HEADER_VERSION_MAX_SIZE, size));
    text_t* url = parse_first(header->text, "^(HTTP\\S+)\\s+", error_out);
    destroy_text(header);
    if (*error_out != NULL)
        return NULL;
    return url;
}

size_t try_get_content_length(char* data_pointer, size_t size)
{
    text_t* header = init_text(data_pointer, MIN(HEADER_HEAP, size));
    error_report_t* error = NULL;
    text_t* content_length_text = parse_first(header->text, "content-length:\\s+(\\S+)\\s+", &error);
    destroy_text(header);
    if (error != NULL)
    {
        destroy_report(error);
        return 0;
    }
    char* end_pointer;
    size_t content_len = strtol(content_length_text->text, &end_pointer, 10);
    destroy_text(content_length_text);
    return content_len;
}

struct header_info* get_response_header(char* data_pointer, size_t size, error_report_t** error_out,
        int is_content_size_required)
{
    int code = get_code(data_pointer, size, error_out);
    if (*error_out != NULL)
        return NULL;
    text_t* redirect_url = NULL;
    // 3xx redirection codes
    if (code >= 300 && code < 400)
        redirect_url = get_redirect_url(data_pointer, size, error_out);
    if (*error_out != NULL)
        return NULL;
    text_t* http_version = get_http_version(data_pointer, size, error_out);
    if (*error_out != NULL)
        return NULL;
    struct header_info* header_info = malloc(sizeof(struct header_info));
    if (is_content_size_required)
        header_info->content_length = try_get_content_length(data_pointer, size);
    header_info->redirect_url = redirect_url;
    header_info->response_code = code;
    header_info->http_version = http_version;
    return header_info;
}

void free_http_header(struct header_info **header_ref)
{
    if ((*header_ref)->redirect_url == NULL)
        free((*header_ref)->redirect_url);
    if ((*header_ref)->http_version == NULL)
        free((*header_ref)->http_version);
    free((*header_ref));
    *header_ref = NULL;
}

