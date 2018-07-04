//
// Created by sergei on 14.05.18.
//

#include "simple_list.h"

#include <stddef.h>
#include <stdlib.h>

struct list_container
{
    struct list_container* next_container;
    void* data;
};

struct simple_list
{
    int size;
    /*
     * first container does not count
     */
    struct list_container first_container;
};

int get_list_size(struct simple_list* list)
{
    return list->size;
}

struct simple_list* init_simple_list()
{
    struct simple_list* list = malloc(sizeof(struct simple_list));
    list->size = 0;
    list->first_container.data = NULL;
    list->first_container.next_container = NULL;
    return list;
}
void destroy_simple_list(struct simple_list* list_ref)
{
    while(list_ref->size != 0)
    {
        free(remove_list_element(list_ref, 0));
    }
    free(list_ref);
}

struct list_container* get_container(struct simple_list* list_ref, int pos)
{
    if (pos >= list_ref->size)
        return NULL;
    struct list_container* current_container = &list_ref->first_container;

    for(; pos >= 0; pos--)
    {
        current_container = current_container->next_container;
    }
    return current_container;
}


void insert_list_element(struct simple_list *list_ref, int pos, void *data)
{
    struct list_container* new_container_ref = malloc(sizeof(struct list_container));
    new_container_ref->data = data;
    struct list_container* prev_container_ref = get_container(list_ref, pos - 1);
    new_container_ref->next_container = prev_container_ref->next_container;
    prev_container_ref->next_container = new_container_ref;
    list_ref->size++;
}

void* remove_container(struct list_container* prev_element)
{
    struct list_container* remove_container = prev_element->next_container;
    prev_element->next_container = remove_container->next_container;
    void* data = remove_container->data;
    free(remove_container);
    return data;
}

void* remove_list_element(struct simple_list *list_ref, int pos)
{
    if (pos >= list_ref->size)
        return NULL;
    struct list_container* prev_container;

    prev_container = get_container(list_ref, pos - 1);
    void* data = remove_container(prev_container);

    list_ref->size--;
    return data;
}

void* remove_list_element_by_data(struct simple_list *list, void *data)
{
    struct list_container* prev_container = &list->first_container;
    void* removed_data = NULL;

    while(prev_container->next_container != NULL)
    {
        if (prev_container->next_container->data == data)
        {
            removed_data = remove_container(prev_container);
            list->size--;
            break;
        }
        prev_container = prev_container->next_container;
    }
    return removed_data;
}

void* get_list_element(struct simple_list *list_ref, int pos)
{
    struct list_container* current_container = get_container(list_ref, pos);
    return current_container->data;
}

struct enumerator {
    struct list_container* next_container;
};

struct enumerator* get_enumerator(struct simple_list* list_ref)
{
    struct enumerator* enumerator_ref = malloc(sizeof(struct enumerator));
    enumerator_ref->next_container = &list_ref->first_container;
    return enumerator_ref;
}
void* get_next_element(struct enumerator* enumerator_ref)
{
    if (enumerator_ref->next_container->next_container == NULL)
        return NULL;
    enumerator_ref->next_container = enumerator_ref->next_container->next_container;
    return enumerator_ref->next_container->data;
}