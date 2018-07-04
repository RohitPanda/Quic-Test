//
// Created by sergei on 02.06.18.
//

#ifndef QUIC_PROBE_MONO_PARSER_H

#include "error_report.h"

/*
 * find first occurrence of "()" with regex operator
 */
text_t* parse_first(char * text, const char* regex_pattern, error_report_t** error_ref_out);


#define QUIC_PROBE_MONO_PARSER_H

#endif //QUIC_PROBE_MONO_PARSER_H
