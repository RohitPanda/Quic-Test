//
// Created by sergei on 12.06.18.
//

#include <stdlib.h>
#include "queue.h"

struct queue_container* init_empty_container()
{
    struct queue_container* container = malloc(sizeof(struct queue_container));
    container->next_container = NULL;
    container->previous_container = NULL;
    container->data = NULL;
    return container;
}

struct queue_container* init_container_with_data(struct queue_container* prev, struct queue_container* next, void* data)
{
    struct queue_container* container = malloc(sizeof(struct queue_container));
    container->next_container = next;
    container->previous_container = prev;
    container->data = data;
    return container;
}

void destroy_container(struct queue_container* queue_container)
{
    free(queue_container);
}

void queue_put_data(struct queue* queue, void* data){
    struct queue_container* container = init_container_with_data(queue->start, queue->start->next_container, data);
    queue->start->next_container->previous_container = container;
    queue->start->next_container = container;
    queue->length++;
}

void* queue_pull_data(struct queue* queue){
    if (queue->length == 0)
        return NULL;
    struct queue_container* last_container = queue->end->previous_container;
    last_container->previous_container->next_container = queue->end;
    queue->end->previous_container = last_container->previous_container;
    void* data = last_container->data;
    destroy_container(last_container);
    queue->length--;
    return data;
}


struct queue* init_queue()
{
    struct queue* queue = malloc(sizeof(struct queue));
    queue->length = 0;
    queue->start = init_empty_container();
    queue->end = init_empty_container();
    queue->start->next_container = queue->end;
    queue->end->previous_container = queue->start;
    return queue;
}
void destroy_queue(struct queue* queue)
{
    while(queue->length != 0)
        queue_pull_data(queue);
    destroy_container(queue->start);
    destroy_container(queue->end);
    free(queue);
}