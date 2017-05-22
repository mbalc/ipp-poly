typedef struct PointerStack PointerStack;

PointerStack NewPointerStack();

void PushOntoStack(void *new_element, PointerStack *stack);

void* GetStackTop(PointerStack *stack);

void PopStack(PointerStack *stack);
