//
// Created by sergei on 14.05.18.
//

#ifndef QUIC_PROBE_STACK_H
#define QUIC_PROBE_STACK_H
#include <sys/types.h>

struct primitive_stack;

struct primitive_stack* init_stack();
void destroy_stack(struct primitive_stack *stack_ref);
void put_stack_element(struct primitive_stack *primitive_stack_ref, void *data);
void* pull_stack_element(struct primitive_stack *primitive_stack_ref);
uint get_stack_length(struct primitive_stack *stack_ref);

#endif //QUIC_PROBE_STACK_H
