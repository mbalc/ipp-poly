#ifndef __STACK_H__
#define __STACK_H__

#include <stdlib.h>
#include <stdbool.h>


typedef struct PointerStack
{
    void *elem_pointer;
    struct PointerStack *next_elem;
    unsigned size;
} PointerStack;

PointerStack NewPointerStack();

void PushOntoStack(void *new_element, PointerStack *stack);

bool HasStackTop(PointerStack *stack);

void* GetStackTop(PointerStack *stack);

void PopStack(PointerStack *stack);

#endif /* __STACK_H__ */
