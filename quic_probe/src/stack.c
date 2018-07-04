//
// Created by sergei on 14.05.18.
//

#include "stack.h"

#include <stddef.h>
#include <stdlib.h>


struct stack_node {
    void* data;
    struct stack_node* next_node;
};

struct primitive_stack
{
    int elements_left;
    struct stack_node* next_node;
};

struct primitive_stack* init_stack()
{
    struct primitive_stack* primitive_stack = malloc(sizeof(struct primitive_stack));
    primitive_stack->elements_left = 0;
    primitive_stack->next_node = NULL;
    return primitive_stack;
};

void destroy_stack(struct primitive_stack *stack_ref)
{
    int has_values = 1;
    while(has_values)
    {
        void* data = pull_stack_element(stack_ref);
        if (data)
        {
            free(data);
        } else
            has_values = 0;
    }
    free(stack_ref);
}

void put_stack_element(struct primitive_stack *primitive_stack_ref, void *data)
{
    struct stack_node* node_ref = malloc(sizeof(struct stack_node));
    node_ref->data = data;
    node_ref->next_node = primitive_stack_ref->next_node;

    primitive_stack_ref->next_node = node_ref;
    primitive_stack_ref->elements_left++;
}

void* pull_stack_element(struct primitive_stack *primitive_stack_ref)
{
    if (primitive_stack_ref->elements_left == 0)
        return NULL;
    struct stack_node* node_ref = primitive_stack_ref->next_node;

    primitive_stack_ref->next_node = node_ref->next_node;
    primitive_stack_ref->elements_left--;

    void* data_ref = node_ref->data;
    free(node_ref);
    return data_ref;
}

uint get_stack_length(struct primitive_stack *stack_ref){
    return stack_ref->elements_left;
}
