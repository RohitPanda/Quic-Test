//
// Created by sergei on 07.05.18.
//

#include "connection_params.h"

#include <string.h>
#include <stdlib.h>

#include "engine_structs.h"

connection_parameters* find_connection(struct simple_list* connection_list, const char* hostname,
        int is_dedicated_stream)
{
    struct enumerator* enumerator_ref = get_enumerator(connection_list);
    connection_parameters* connection = (connection_parameters*)get_next_element(enumerator_ref);
    while(connection != NULL)
    {
        if (strcmp(hostname, connection->hostname->text) == 0)
            if ((get_list_size(connection->stream_list) == 0) || !is_dedicated_stream)
                break;
        connection = (connection_parameters*)get_next_element(enumerator_ref);
    }
    free(enumerator_ref);
    return connection;
}