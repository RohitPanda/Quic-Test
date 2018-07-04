//
// Created by sergei on 15.06.18.
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "arguments_parser.h"

#include "helper.h"

static bool check_ip(const char* text, enum IPv* ip_version_out)
{
    if(strcmp(text, "-4")==0)
    {
        *ip_version_out = IPv4;
        return true;
    }

    else if(strcmp(text, "-6")==0)
    {
        *ip_version_out = IPv6;
        return true;
    }
    return false;
}

static bool check_time(int* current_index_ref, char **argv, int* mintime_out, int* maxtime_out)
{
    if(strcmp(argv[*current_index_ref], "--maxtime")==0)
    {
        *maxtime_out = atoi(argv[++(*current_index_ref)]);
        return true;
    }
    if(strcmp(argv[*current_index_ref], "--mintime")==0)
    {
        *mintime_out = atoi(argv[++(*current_index_ref)]);
        return true;
    }
    return false;
}

static bool check_one_bitrate(const char* text, bool* one_bitrate_out)
{
    if(strcmp(text, "--onebitrate")==0)
    {
        *one_bitrate_out=true;
        return true;
    }
    return false;
}

static bool check_instantaneous_output(const char* text, bool* instantaneous_output_out)
{
    if(strcmp(text, "--verbose")==0)
    {
        *instantaneous_output_out=true;
        return true;
    }
    return false;
}

static bool check_max_bitrate(int* current_index_ref, char **argv, int* max_bitrate)
{
    if(strcmp(argv[*current_index_ref], "--maxbitrate")==0)
    {
        *max_bitrate = atoi(argv[++(*current_index_ref)]) / 8;
        return true;
    }
    return false;
}

static bool check_buffer_len(int* current_index_ref, char **argv, int* buffer_len)
{
    if(strcmp(argv[*current_index_ref], "--range")==0)
    {
        /*value must be greater than LEN_CHUNK_MINIMUM seconds*/
        *buffer_len = atoi(argv[++(*current_index_ref)]);
        if(*buffer_len<LEN_CHUNK_MINIMUM)
            *buffer_len = LEN_CHUNK_MINIMUM;
        return true;
    }
    return false;
}

static bool check_help(const char* text, char **argv)
{
    if (strcmp(text, "--help") == 0)
    {
        printhelp(argv[0]);
        return true;
    }
    return false;
}

static bool check_output_help(const char* text)
{
    if (strcmp(text, "--help_output") == 0)
    {
        print_help_output();
        return true;
    }
    return false;
}

bool check_url_existance(const char *youtube_link, char **argv)
{
    if(strlen(youtube_link)==0)
    {
        printf("Youtbe URL not detected\n");
        printf("To print help use the program --help switch : %s --help\n", argv[0]);
        return false;
    }
    return true;
}

uint find_char_occurance(char* text, char symbol, uint* symbol_positions)
{
    uint counter = 0;
    for(uint i = 0; i < strlen(text); i++)
    {
        if (text[i] == symbol)
        {
            symbol_positions[counter++] = i;
        }
    }
    return counter;
}

#define PORT_STRING_LENGTH 6

uint parse_array(char* text, uint** array_out)
{
    uint comma_positions[10];
    uint comma_count = find_char_occurance(text, ',', comma_positions);
    uint* parsed_values = calloc(sizeof(uint), comma_count + 1);
    // 65535\0 max port
    char port_number_string[PORT_STRING_LENGTH];
    uint previous_post_comma_position = 0;
    for(int i = 0; i < comma_count; i++)
    {
        memzero(port_number_string, PORT_STRING_LENGTH);
        strncpy(port_number_string, text + previous_post_comma_position,
                comma_positions[i] - previous_post_comma_position);
        previous_post_comma_position = comma_positions[i] + 1;
        parsed_values[i] = atoi(port_number_string);
    }
    memzero(port_number_string, PORT_STRING_LENGTH);
    strncpy(port_number_string, text + previous_post_comma_position, strlen(text) - previous_post_comma_position);
    parsed_values[comma_count] = atoi(port_number_string);
    *array_out = parsed_values;
    return comma_count + 1;
}

int set_arguments(int argc, char **argv, char *youtubelink, struct program_arguments* arguments_out)
{
    for(int i=1; i<argc; i++)
    {
        if(check_help(argv[i], argv) || check_output_help(argv[i]))
        {
            return 0;
        }

        if (check_ip(argv[i], &arguments_out->ip_version))
            continue;
        if (check_time(&i, argv, &arguments_out->min_test_time, &arguments_out->max_test_time))
            continue;
        if (check_one_bitrate(argv[i], &arguments_out->one_bitrate))
            continue;
        if (check_instantaneous_output(argv[i], &arguments_out->instantaneous_output))
            continue;
        if (check_max_bitrate(&i, argv, &arguments_out->max_bitrate))
            continue;
        if (check_buffer_len(&i, argv, &arguments_out->buffer_len))
            continue;
        else if(strstr(argv[i], "youtube")!=NULL && strstr(argv[i], "watch")!=NULL )
        {
            if(strlen(argv[i])>MAXURLLENGTH)
            {
                printf("Youtbe URL provided as argument is too long\n");
                return 0;
            }
            else
                strcpy(youtubelink, argv[i]);
        }
    }
    return check_url_existance(youtubelink, argv);
}