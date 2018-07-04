//
// Created by sergei on 14.05.18.
//

#ifndef QUIC_PROBE_SIMPLE_LIST_H
#define QUIC_PROBE_SIMPLE_LIST_H

struct simple_list;
struct enumerator;

struct simple_list* init_simple_list();
void destroy_simple_list(struct simple_list* list_ref);

int get_list_size(struct simple_list* list);
void insert_list_element(struct simple_list *list_ref, int pos, void *data);
void* remove_list_element(struct simple_list *list_ref, int pos);
void* get_list_element(struct simple_list *list_ref, int pos);
void* remove_list_element_by_data(struct simple_list *list, void *data);

struct enumerator* get_enumerator(struct simple_list* list_ref);
void* get_next_element(struct enumerator* enumerator_ref);

#endif //QUIC_PROBE_SIMPLE_LIST_H
