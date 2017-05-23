#include <stdlib.h>


typedef struct PointerStack
{
    void *elem_pointer;
    struct PointerStack *next_elem;
    unsigned size;
} PointerStack;

PointerStack NewPointerStack()
{
    return (PointerStack) {.elem_pointer = NULL, .next_elem = NULL, .size = 0};
}

void PushOntoStack(void *new_element, PointerStack *stack)
{
    PointerStack *memory = malloc(sizeof(PointerStack));
    *memory = *stack;
    stack->next_elem = memory;
    stack->elem_pointer = new_element;
    stack->size = memory->size + 1;
}

void* GetStackTop(PointerStack *stack)
{
    return stack->elem_pointer;
}

void PopStack(PointerStack *stack)
{
    PointerStack *memory = stack->next_elem;
    *stack = *(stack->next_elem);
    free(memory);
}
