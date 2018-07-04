//
// Created by sergei on 02.06.18.
//
#include <stddef.h>
#include <regex.h>
#include <stddef.h>
#include <args_data.h>

#include "mono_parser.h"
#include "text.h"

#define MAXMATCH 2

text_t* parse_first(char * text, const char* regex_pattern, error_report_t** error_ref_out)
{
    regex_t regex;
    regmatch_t matches[MAXMATCH];
    int status;

    status = regcomp(&regex, regex_pattern, REG_EXTENDED);
    if (status != 0)
    {
        *error_ref_out = create_report(init_text_from_const("Could not compile regex\n"), 0, __FILE__, __LINE__,
                QUIC_CLIENT_CODE_BAD_RESPONSE);
        return NULL;
    }
    status |= regexec(&regex, text, MAXMATCH, matches, 0);
    if (status)
    {
        *error_ref_out = create_report(init_text_from_const("Search text was not found"), 0, __FILE__, __LINE__,
                QUIC_CLIENT_CODE_BAD_RESPONSE);
        return NULL;
    }
    text_t * return_text = init_text(text + matches[1].rm_so, (size_t)(matches[1].rm_eo - matches[1].rm_so));
    regfree(&regex);
    return return_text;
}
