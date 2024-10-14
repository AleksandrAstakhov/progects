#pragma once

#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct node {
  uintptr_t value;
  struct node *next;
} node_t;

typedef struct lfstack {
  node_t *top;
} lfstack_t;

int lfstack_init(lfstack_t *stack) {
  stack->top = NULL;
  return 0;
}

int lfstack_push(lfstack_t *stack, uintptr_t value) {
  lfstack_t origin_stack = atomic_load(stack);
  node_t *new_node = (node_t *)malloc(sizeof(node_t));
  new_node->value = value;
  new_node->next = origin_stack.top;
  while (
      !atomic_compare_exchange_weak(&stack->top, &origin_stack.top, new_node)) {
    new_node->next = origin_stack.top;
  }
  return 0;
}

int lfstack_pop(lfstack_t *stack, uintptr_t *value) {
  lfstack_t origin_stack = atomic_load(stack);
  if (origin_stack.top == NULL) {
    *value = 0;
    return 0;
  }
  *value = origin_stack.top->value;
  while (!atomic_compare_exchange_weak(&stack->top, &origin_stack.top,
                                       origin_stack.top->next)) {
  }
  free(origin_stack.top);
  return 0;
}

int lfstack_destroy(lfstack_t *stack) { return 0; }
