//
// Created by sergei on 12.06.18.
//

#ifndef QUIC_PROBE_QUEUE_H
#define QUIC_PROBE_QUEUE_H

struct queue{
    struct queue_container* start;
    struct queue_container* end;
    int length;
};

struct queue_container{
    void* data;
    struct queue_container* previous_container;
    struct queue_container* next_container;
};

void queue_put_data(struct queue* queue, void* data);
void* queue_pull_data(struct queue* queue);

struct queue* init_queue();
void destroy_queue(struct queue* queue);

#endif //QUIC_PROBE_QUEUE_H
