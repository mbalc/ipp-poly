/** @file
   Implementacja kalkulatora działającego na wielomianach

   @author Michał Balcerzak <mb385130@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-27
 */

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <assert.h>
#include "poly.h"
#include "stack.h"

static char global_pcalc_read_buffer;
static PointerStack global_pcalc_poly_stack;

static unsigned global_pcalc_line_number;
static unsigned global_pcalc_column_number;


/**
 * Alokuje na stercie (ang - heap) miejsce na strukturę Mono.
 * @return wskaźnik na nowy obszar w pamięci
 */
static inline Mono* MonoMalloc()
{
    Mono *out = (Mono*)malloc(sizeof(Mono));
    assert(out);
    return out;

}
/**
 * Alokuje na stercie (ang - heap) miejsce na strukturę Poly.
 * @return wskaźnik na nowy obszar w pamięci
 */
static inline Poly* PolyMalloc()
{
    Poly *out = (Poly*)malloc(sizeof(Poly));
    assert(out);
    return out;
}


void MonoStackDestroy(PointerStack *mono_stack)
{
    while (mono_stack->next_elem != NULL)
    {
        free(mono_stack->elem_pointer);
        PopStack(mono_stack);
    }
}

void PolyStackDestroy(PointerStack *poly_stack)
{
    while (poly_stack->next_elem != NULL)
    {
        PolyDestroy(poly_stack->elem_pointer);
        free(poly_stack->elem_pointer);
        PopStack(poly_stack);
    }
}


bool BufferIsNumber()
{
    return (('0' <= global_pcalc_read_buffer) &&
            (global_pcalc_read_buffer <= '9')) ||
           global_pcalc_read_buffer == '-';
}

bool BufferIsEndline()
{
    return global_pcalc_read_buffer == '\n' ||
           global_pcalc_read_buffer == EOF;
}

bool BufferIsLetter()
{
    return ('a' <= global_pcalc_read_buffer &&
            global_pcalc_read_buffer <= 'z') ||
           ('A' <= global_pcalc_read_buffer &&
            global_pcalc_read_buffer <= 'Z');
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
    while (!BufferIsEndline())
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
    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", global_pcalc_line_number);
    return true;
}

bool ThrowParsePolyError()
{
    fprintf(stderr, "ERROR %d %d\n",
            global_pcalc_line_number, global_pcalc_column_number);
    return true;
}

bool ThrowParseCommandError()
{
    fprintf(stderr, "ERROR %d WRONG COMMAND\n", global_pcalc_line_number);
    return true;
}

bool ThrowParseAtArgError()
{
    fprintf(stderr, "ERROR %d WRONG VALUE\n", global_pcalc_line_number);
    return true;
}

bool ThrowParseDegByArgError()
{
    fprintf(stderr, "ERROR %d WRONG VARIABLE\n", global_pcalc_line_number);
    return true;
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
    printf("\n");
}

void StackTopInsertZero()
{
    Poly *new_poly = PolyMalloc();
    *new_poly = PolyZero();
    PushOntoStack(new_poly, &global_pcalc_poly_stack);
}

void StackTopClone()
{
    Poly *new_poly = PolyMalloc();
    *new_poly = PolyClone(GetStackTop(&global_pcalc_poly_stack));
    PushOntoStack(new_poly, &global_pcalc_poly_stack);
}

void StackTopPop()
{
    Poly *a = global_pcalc_poly_stack.elem_pointer;
    PopStack(&global_pcalc_poly_stack);
    PolyDestroy(a);
    free(a);
}

void StackTopNeg()
{
    Poly *a = PollStackTop(&global_pcalc_poly_stack);
    Poly *b = PolyMalloc();
    *b = PolyNeg(a);
    PolyDestroy(a);
    free(a);
    PushOntoStack(b, &global_pcalc_poly_stack);
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
    Poly *res = PolyMalloc();
    *res = operation(a, b);
    PushOntoStack(res, &global_pcalc_poly_stack);
    PolyDestroy(a);
    free(a);
    PolyDestroy(b);
    free(b);
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

void StackTopAt(poly_coeff_t x)
{
    Poly *a = PollStackTop(&global_pcalc_poly_stack);
    Poly *b = PolyMalloc();
    *b = PolyAt(a, x);
    PolyDestroy(a);
    free(a);
    PushOntoStack(b, &global_pcalc_poly_stack);
}

bool AddNumbers(long lower_limit, long upper_limit, long *a, long b)
{
    if (*a >= 0)
    {
        if (upper_limit - *a < b)
        {
            return true;
        }
    }
    if (*a <= 0)
    {
        if (b < lower_limit - *a)
        {
            return true;
        }
    }
    *a = *a + b;
    return false;
}

bool MultiplyByTen(long lower_limit, long upper_limit, long *a)
{
    for (int i = 2; i < 16; i = i + i)
    {
        if (AddNumbers(lower_limit, upper_limit, a, *a))
        {
            return true;
        }
        //a = i * a
    }
    //a = 8 * a;
    return AddNumbers(lower_limit, upper_limit, a, *a / 4);
}


bool ParseNumber(long lower_limit, long upper_limit, long *output)
{
    long out = 0;
    bool negative = (global_pcalc_read_buffer == '-');
    if (negative)
    {
        ReadCharacter(); //pomijam minus
    }
    while (BufferIsNumber())
    {
        if (MultiplyByTen(lower_limit, upper_limit, &out))
        {
            return true;
        }
        if (AddNumbers(lower_limit, upper_limit, &out,
                       (negative ? -1 : 1) * (global_pcalc_read_buffer - '0')))
        {
            return true;
        }
        ReadCharacter();
    }
    *output = out;
    return false;
}

bool ParseCoeff(poly_coeff_t *out)
{
    long parser_output;
    if (ParseNumber(LONG_MIN, LONG_MAX, &parser_output))
    {
        return true;
    }
    *out = parser_output;
    return false;
}

bool ParseExp(poly_exp_t *out)
{
    long parser_output;
    if (ParseNumber(0, INT_MAX, &parser_output))
    {
        return true;
    }
    *out = parser_output;
    return false;
}

bool ExecuteOnPolyStack(unsigned size_requirement, void (*procedure)())
{
    if (global_pcalc_poly_stack.size >= size_requirement)
    {
        procedure();
        return false;
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
    bool status = scanf("%20s", command + 1); //max_command_length used
    assert(status);
    ReadCharacter();
    if (BufferIsEndline())
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
        else if (strcmp(command, "NEG") == 0)
        {
            return ExecuteOnPolyStack(1, StackTopNeg);
        }
        else if (strcmp(command, "IS_EQ") == 0)
        {
            return ExecuteOnPolyStack(2, StackTopIsEq);
        }
        else if (strcmp(command, "ADD") == 0)
        {
            return ExecuteOnPolyStack(2, StackTopAdd);
        }
        else if (strcmp(command, "MUL") == 0)
        {
            return ExecuteOnPolyStack(2, StackTopMul);
        }
        else if (strcmp(command, "SUB") == 0)
        {
            return ExecuteOnPolyStack(2, StackTopSub);
        }
        else
        {
            return ThrowParseCommandError();
        }
    }
    if (global_pcalc_read_buffer == ' ')
    {
        ReadCharacter();
        if (strcmp(command, "AT") == 0)
        {
            if (!BufferIsNumber())
            {
                return ThrowParseAtArgError();
            }
            poly_coeff_t arg;
            if (ParseCoeff(&arg))
            {
                return ThrowParseAtArgError();
            }
            if (global_pcalc_read_buffer != '\n')
            {
                return ThrowParseAtArgError();
            }
            if (global_pcalc_poly_stack.size >= 1)
            {
                StackTopAt(arg);
                return false;
            }
            else
            {
                return ThrowStackUnderflow();
            }
        }
        else if (strcmp(command, "DEG_BY") == 0)
        {
            if (!BufferIsNumber())
            {
                return ThrowParseDegByArgError();
            }
            long arg;
            if (ParseNumber(0, UINT_MAX,&arg))
            {
                return ThrowParseDegByArgError();
            }
            if (global_pcalc_read_buffer != '\n')
            {
                return ThrowParseDegByArgError();
            }
            unsigned idx = arg;
            if (global_pcalc_poly_stack.size >= 1)
            {
                StackTopDegBy(idx);
                return false;
            }
            else
            {
                return ThrowStackUnderflow();
            }
        }
        else
        {
            return ThrowParseCommandError();
        }

    }
    return ThrowParseCommandError();
}

bool ParsePoly(Poly *output);

bool ParseMono(Mono *output)
{
    Poly *poly_coeff = PolyMalloc();
    if (ParsePoly(poly_coeff))
    {
        free(poly_coeff);
        return true;
    }
    if (global_pcalc_read_buffer != ',')
    {
        PolyDestroy(poly_coeff);
        free(poly_coeff);
        return true;
    }
    ReadCharacter();
    poly_exp_t e;
    if (!BufferIsNumber())
    {
        PolyDestroy(poly_coeff);
        free(poly_coeff);
        return true;
    }
    if (ParseExp(&e))
    {
        PolyDestroy(poly_coeff);
        free(poly_coeff);
        return true;
    }
    if (global_pcalc_read_buffer != ')')
    {
        PolyDestroy(poly_coeff);
        free(poly_coeff);
        return true;
    }
    ReadCharacter();
    *output = MonoFromPoly(poly_coeff, e);
    free(poly_coeff);
    return false;
}

bool ParsePoly(Poly *output)
{
    if (global_pcalc_read_buffer == '(')
    {
        PointerStack mono_stack = NewPointerStack();
        while (global_pcalc_read_buffer == '(')
        {
            Mono *new_mono = MonoMalloc();
            ReadCharacter();
            if (ParseMono(new_mono))
            {
                free(new_mono);
                MonoStackDestroy(&mono_stack);
                return true;
            }
            PushOntoStack(new_mono, &mono_stack);
            if (BufferIsEndline() || global_pcalc_read_buffer == ',')
            {
                break;
            }
            else if (global_pcalc_read_buffer == '+')
            {
                ReadCharacter();
                if (global_pcalc_read_buffer != '(')
                {
                    MonoStackDestroy(&mono_stack);
                    return true;
                }
            }
            else
            {
                MonoStackDestroy(&mono_stack);
                return true;
            }
        }
        unsigned monos_size = mono_stack.size;
        Mono monos[monos_size];
        for (PointerStack *ptr = &mono_stack; ptr->next_elem != NULL;
             ptr = ptr->next_elem)
        {
            monos[ptr->size - 1] = *(Mono*)(GetStackTop(ptr));
        }
        MonoStackDestroy(&mono_stack);
        *output = PolyAddMonos(monos_size, monos);
    }
    else if (BufferIsNumber())
    {
        poly_coeff_t coeff;
        if (ParseCoeff(&coeff))
        {
            return true;
        }
        *output = PolyFromCoeff(coeff);

    }
    else
    {
        return true;
    }
    return false;
}


void ParseLine()
{
    ReadCharacter();
    if (BufferIsLetter())
    {
        if (ParseCommand())
        {
            ReadUntilNewline();
        }
    }
    else if (BufferIsEndline())
    {
    }
    else
    {
        Poly *new_poly = PolyMalloc();
        if (ParsePoly(new_poly) || !BufferIsEndline())
        {
            free(new_poly);
            ThrowParsePolyError();
            ReadUntilNewline();
        }
        else
        {
            PushOntoStack(new_poly, &global_pcalc_poly_stack);
        }
    }
    if (!BufferIsEndline())
    {
        assert(false);
    }
}

int main()
{
    global_pcalc_poly_stack = NewPointerStack();
    global_pcalc_read_buffer = 1;
    global_pcalc_line_number = 1;
    global_pcalc_column_number = 0;
    while (global_pcalc_read_buffer != EOF)
    {
        ParseLine();
    }
    PolyStackDestroy(&global_pcalc_poly_stack);
    return 0;
}
