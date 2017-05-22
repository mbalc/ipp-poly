typedef struct PointerStack
{
    void *elem_pointer;
    struct PointerStack *next_elem;
} PointerStack;

PointerStack NewPointerStack();

void PushOntoStack(void *new_element, PointerStack *stack);

void* GetStackTop(PointerStack *stack);

void PopStack(PointerStack *stack);
