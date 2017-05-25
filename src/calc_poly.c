#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "poly.h"
#include "stack.h"

static char global_pcalc_read_buffer;
static PointerStack global_pcalc_poly_stack;

static unsigned global_pcalc_line_number;
static unsigned global_pcalc_column_number;

void MonoStackDestroy(PointerStack *mono_stack)
{
    while (mono_stack->next_elem != NULL)
    {
        MonoDestroy(mono_stack->elem_pointer);
        PopStack(mono_stack);
    }
    free(mono_stack);
}


bool BufferIsNumber()
{
    return (('0' <= global_pcalc_read_buffer) &&
            (global_pcalc_read_buffer <= '9')) ||
           global_pcalc_read_buffer == '-';
}

bool BufferIsEndline()
{
    return global_pcalc_read_buffer == '\n';
}

void ReadCharacter()
{
    if (global_pcalc_read_buffer == '\n')
    {
        global_pcalc_line_number += 1;
        global_pcalc_column_number = 1;
    }
    else
    {
        global_pcalc_column_number += 1;
    }
    global_pcalc_read_buffer = getchar();
}

void ReadUntilNewline()
{
    while (global_pcalc_read_buffer != '\n'
           && global_pcalc_read_buffer != 0)
    {
        ReadCharacter();
    }
}

void PrintExpressionResult(int expression)
{
    printf("%d\n", expression);
}

bool ThrowStackUnderflow()
{
    printf("ERROR %d STACK UNDERFLOW\n", global_pcalc_line_number);
    return false;
}

void StackTopIsZero()
{
    PrintExpressionResult(PolyIsZero(GetStackTop(&global_pcalc_poly_stack)));
}

void StackTopIsCoeff()
{
    PrintExpressionResult(PolyIsCoeff(GetStackTop(&global_pcalc_poly_stack)));
}

void StackTopDeg()
{
    PrintExpressionResult(PolyDeg(GetStackTop(&global_pcalc_poly_stack)));
}

void StackTopDegBy(unsigned idx)
{
    PrintExpressionResult(PolyDegBy(GetStackTop(&global_pcalc_poly_stack), idx));
}

void StackTopPrint()
{
    PrintPoly(GetStackTop(&global_pcalc_poly_stack));
}

void StackTopInsertZero()
{
    Poly new_poly = PolyZero();
    PushOntoStack(&new_poly, &global_pcalc_poly_stack);
}

void StackTopClone()
{
    Poly new_poly = PolyClone(GetStackTop(&global_pcalc_poly_stack));
    PushOntoStack(&new_poly, &global_pcalc_poly_stack);
}

void StackTopPop()
{
    PopStack(&global_pcalc_poly_stack);
}

void StackTopNeg()
{

}

void StackTopIsEq()
{
    Poly *a = PollStackTop(&global_pcalc_poly_stack);
    Poly *b = GetStackTop(&global_pcalc_poly_stack);
    PushOntoStack(a, &global_pcalc_poly_stack);
    PrintExpressionResult(PolyIsEq(a, b));
}

void PushBinaryPolyOperationResultOntoStack
    (Poly (*operation)(const Poly *a, const Poly *b))
{
    Poly *a = PollStackTop(&global_pcalc_poly_stack);
    Poly *b = PollStackTop(&global_pcalc_poly_stack);
    Poly res = operation(a, b);
    PushOntoStack(&res, &global_pcalc_poly_stack);
    PolyDestroy(a);
    PolyDestroy(b);
}

void StackTopAdd()
{
    PushBinaryPolyOperationResultOntoStack(PolyAdd);
}

void StackTopMul()
{
    PushBinaryPolyOperationResultOntoStack(PolyMul);
}

void StackTopSub()
{
    PushBinaryPolyOperationResultOntoStack(PolySub);
}


bool AddNumbers(long lower_limit, long upper_limit, long *a, long b)
{
    if (*a > 0)
    {
        if (upper_limit - *a < b)
        {
            printf("upperlimit\n");
            return false;
        }
    }
    if (*a < 0)
    {
        printf("%ld < %ld\n", *a - lower_limit, -b);
        if (b < lower_limit - *a)
        {
            printf("lowerlimit\n");
            return false;
        }
    }
    *a = *a + b;
    return true;
}

bool MultiplyByTen(long lower_limit, long upper_limit, long *a)
{
    for (int i = 2; i < 16; i = i + i)
    {
        if (!AddNumbers(lower_limit, upper_limit, a, *a))
        {
            return false;
        }
        //a = i * a
    }
    //a = 8 * a;
    return AddNumbers(lower_limit, upper_limit, a, *a / 4);
}


bool ParseNumber(long lower_limit, long upper_limit, void *output)
{
    long out = 0;
    bool negative = (global_pcalc_read_buffer == '-');
    if (negative)
    {
        ReadCharacter(); //pomijam minus
    }
    while (BufferIsNumber())
    {
        if (!MultiplyByTen(lower_limit, upper_limit, &out))
        {
            return false;
        }
        if (!AddNumbers(lower_limit, upper_limit, &out,
                        (negative ? -1 : 1) * (global_pcalc_read_buffer - '0')))
        {
            return false;
        }
        ReadCharacter();
    }
    if (negative)
    {
        if (out == LONG_MAX)
        {
            return false;
        }
        if (-out < lower_limit)
        {
            return false;
        }
        out = -out;
    }
    *(long*)output = out;
    return true;
}

bool ParseCoeff(poly_coeff_t *out)
{
    return ParseNumber(LONG_MIN, LONG_MAX, out);
}

bool ParseExp(poly_exp_t *out)
{
    return ParseNumber(0, INT_MAX, out);
}

bool ExecuteOnPolyStack(unsigned size_requirement, void (*procedure)())
{
    if (global_pcalc_poly_stack.size >= size_requirement)
    {
        procedure();
        return true;
    }
    else
    {
        return ThrowStackUnderflow();
    }
}

bool ParseCommand()
{
    static unsigned max_command_length = 20;
    char command[max_command_length];
    command[0] = global_pcalc_read_buffer;
    scanf("%s", command + 1);
    printf("%s -<<\n", command);
    ReadCharacter();
    printf("(%c) - buffer\n", global_pcalc_read_buffer);
    if (global_pcalc_read_buffer == '\n')
    {
        if (strcmp(command, "DEG") == 0)
        {
            return ExecuteOnPolyStack(1, StackTopDeg);
        }
        if (strcmp(command, "IS_COEFF") == 0)
        {
            return ExecuteOnPolyStack(1, StackTopIsCoeff);
        }
        else if (strcmp(command, "IS_ZERO") == 0)
        {
            return ExecuteOnPolyStack(1, StackTopIsZero);
        }
        else if (strcmp(command, "PRINT") == 0)
        {
            return ExecuteOnPolyStack(1, StackTopPrint);
        }
        else if (strcmp(command, "ZERO") == 0)
        {
            return ExecuteOnPolyStack(0, StackTopInsertZero);
        }
        else if (strcmp(command, "CLONE") == 0)
        {
            return ExecuteOnPolyStack(1, StackTopClone);
        }
        else if (strcmp(command, "POP") == 0)
        {
            return ExecuteOnPolyStack(1, StackTopPop);
        }
        else
        {
            return false;
        }
        return true;
    }
    if (global_pcalc_read_buffer == ' ')
    {

    }
    return false;
}

bool ParsePoly(Poly *output);

bool ParseMono(Mono *output)
{
    Poly poly_coeff;
    if (!ParsePoly(&poly_coeff))
    {
        return false;
    }
    if (global_pcalc_read_buffer != ',')
    {
        return false;
    }
    ReadCharacter();
    poly_exp_t e;
    if (!ParseExp(&e))
    {
        return false;
    }
    if (global_pcalc_read_buffer != ')')
    {
        return false;
    }
    ReadCharacter();
    *output = MonoFromPoly(&poly_coeff, e);
    return true;
}

bool ParsePoly(Poly *output)
{
    if (global_pcalc_read_buffer == '(')
    {
        PointerStack mono_stack = NewPointerStack();
        while (global_pcalc_read_buffer == '(')
        {
            Mono *new_mono = malloc(sizeof(Mono));
            ReadCharacter();
            if (!ParseMono(new_mono))
            {
                return false;
            }
            PushOntoStack(new_mono, &mono_stack);
            printf("buffer == %c\n", global_pcalc_read_buffer);
            if (global_pcalc_read_buffer == '+')
            {
                ReadCharacter();
            }
        }
        printf("hehe sajz %d\n", mono_stack.size);
        unsigned monos_size = mono_stack.size;
        Mono monos[monos_size];
        for (unsigned i = 0; i < monos_size; ++i)
        {
            monos[i] = *(Mono*)GetStackTop(&mono_stack);
            PopStack(&mono_stack);
        }
        *output = PolyAddMonos(monos_size, monos);
        PrintPoly(output);

    }
    else if (BufferIsNumber())
    {
        poly_coeff_t coeff;
        if (!ParseCoeff(&coeff))
        {
            return false;
        }
        *output = PolyFromCoeff(coeff);

    }
    else
    {
        return false;
    }
    return true;
}


void ParseLine()
{
    ReadCharacter();
    printf("%c ??\n", global_pcalc_read_buffer);
    if (('a' <= global_pcalc_read_buffer && global_pcalc_read_buffer <= 'z') ||
        ('A' <= global_pcalc_read_buffer && global_pcalc_read_buffer <= 'Z'))
    {
        if (!ParseCommand())
        {
            printf("ERROR\n");
            ReadUntilNewline();
        }
    }
    else
    {
        Poly *new_poly = malloc(sizeof(Poly));
        if (!ParsePoly(new_poly))
        {
            printf("ERROR %d %d\n", global_pcalc_line_number,
                   global_pcalc_column_number);
            ReadUntilNewline();
        }
        PushOntoStack(new_poly, &global_pcalc_poly_stack);
    }
    if (global_pcalc_read_buffer != '\n')
    {
        printf("da (%c) aah!\n", global_pcalc_read_buffer);
    }
}

int main()
{
    global_pcalc_read_buffer = 1;
    while (global_pcalc_read_buffer != 0)
        ParseLine();
    return 0;
}
