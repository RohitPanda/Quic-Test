//
// Created by sergei on 24.05.18.
//

#include "error_report.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>


void copy_message(const char* message, char** dest, int* message_len_out) {
    *message_len_out = strlen(message);
    *dest = malloc(*message_len_out);
    strcpy(*dest, message);
}

void fill_report(error_report_t* report, text_t* error_message, int error_code, const char* filename, int line,
                 int local_error_code) {
    report->error_code = error_code;
    report->error = error_message;
    report->filename = init_text_from_const(filename);
    report->line = line;
    report->local_error_code = local_error_code;
}

error_report_t* init_report() {
    error_report_t* report = malloc(sizeof(error_report_t));
    report->local_error_code = 0;
    return report;
}

error_report_t* create_report(text_t* error_message, int error_code, const char* filename, int line,
                              int local_error_code) {
    error_report_t* report = malloc(sizeof(error_report_t));
    fill_report(report, error_message, error_code, filename, line, local_error_code);
    return report;
}

void destroy_report(error_report_t* report){
    if (!report->local_error_code)
        return;
    destroy_text(report->filename);
    destroy_text(report->error);
}