//
// Created by sergei on 24.05.18.
//

#ifndef QUIC_PROBE_ERROR_REPORT_H
#define QUIC_PROBE_ERROR_REPORT_H


#include "text.h"

typedef struct error_report{
    int local_error_code;
    text_t* error;
    int error_code;
    text_t* filename;
    int line;
} error_report_t;

error_report_t* init_report();

void fill_report(error_report_t* report, text_t* error_message, int error_code, const char* filename, int line,
                 int local_error_code);

error_report_t* create_report(text_t* error_message, int error_code, const char* filename, int line,
                              int local_error_code);

void destroy_report(error_report_t* report);

#endif //QUIC_PROBE_ERROR_REPORT_H
