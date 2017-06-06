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
#include "utils.h"


/** Ostatni wczytany ze standardowego wejścia znak. */
static char global_pcalc_read_buffer;

/** Główny stos wielomianów, na którym operuje kalkulator. */
static PointerStack global_pcalc_poly_stack;

/** Numer wiersza, z którego były ostatnio wczytywane znaki. */
static unsigned global_pcalc_line_number;

/** Numer ostatnio wczytanej kolumny. */
static unsigned global_pcalc_column_number;


/**
 * Alokuje na stercie (ang - heap) miejsce na strukturę Mono.
 * @return wskaźnik na nowy obszar w pamięci
 */
static Mono* MonoMalloc()
{
    Mono *out = (Mono*)malloc(sizeof(Mono));
    assert(out);
    return out;

}
/**
 * Alokuje na stercie (ang - heap) miejsce na strukturę Poly.
 * @return wskaźnik na nowy obszar w pamięci
 */
static Poly* PolyMalloc()
{
    Poly *out = (Poly*)malloc(sizeof(Poly));
    assert(out);
    return out;
}

/**
 * Wczytuje kolejną literę z wejścia do pamięci.
 * Ostatnio wczytany znak jest dostępny dla parserów w globalnej zmiennej.
 */
static void ReadCharacter()
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

/**
 * Usuwa zawartość stosu jednomianów z pamięci.
 * @param[in,out] mono_stack : stos jednomianów, który chcemy wyczyścić
 */
static void MonoStackDestroy(PointerStack *mono_stack)
{
    while (mono_stack->next_elem != NULL)
    {
        free(mono_stack->elem_pointer);
        PopStack(mono_stack);
    }
}

/**
 * Usuwa zawartość stosu wielomianów z pamięci.
 * @param[in,out] poly_stack : stos wielomianów, który chcemy wyczyścić
 */
static void PolyStackDestroy(PointerStack *poly_stack)
{
    while (poly_stack->next_elem != NULL)
    {
        PolyDestroy(poly_stack->elem_pointer);
        free(poly_stack->elem_pointer);
        PopStack(poly_stack);
    }
}

/**
 * Sprawdza, czy ostatnio wczytany znak opisuje liczbę.
 * @return czy bufor jest cyfrą bądź minusem
 */
static bool BufferIsNumber()
{
    return (('0' <= global_pcalc_read_buffer) &&
            (global_pcalc_read_buffer <= '9')) ||
           global_pcalc_read_buffer == '-';
}

/**
 * Sprawdza, czy ostatnio wczytany znak opisuje koniec wiersza lub pliku wejścia.
 * @return czy bufor jest znakiem końcą wiersza bądź oznacza koniec pliku
 */
static bool BufferIsEndline()
{
    return global_pcalc_read_buffer == '\n' ||
           global_pcalc_read_buffer == EOF;
}

/**
 * Sprawdza, czy ostatnio wczytany znak jest literą alfabetu łacińskiego.
 * @return czy bufor jest literą
 */
static bool BufferIsLetter()
{
    return ('a' <= global_pcalc_read_buffer &&
            global_pcalc_read_buffer <= 'z') ||
           ('A' <= global_pcalc_read_buffer &&
            global_pcalc_read_buffer <= 'Z');
}

/**
 * Wczytuje znaki z wejścia póki nie napotka znaku końca wiersza lub znaku EOF.
 * Używane przede wszystkim w przypadku napotkania błędu przez któryś z parserów.
 */
static void ReadUntilNewline()
{
    while (!BufferIsEndline())
    {
        ReadCharacter();
    }
}

/**
 * Wypisuje na standardowe wyjście wartość wyrażenia.
 * @param[in] expression : wyrażenie do wypisania
 */
static void PrintExpressionResult(int expression)
{
    printf("%d\n", expression);
}

/**
 * Zwraca błąd o zbyt małej liczbie wielomianów na stosie i wypisuje odpowiedni
 * komunikat.
 * @return status wykonania dla błędu
 */
static bool ThrowStackUnderflow()
{
    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", global_pcalc_line_number);
    return true;
}

/**
 * Zwraca błąd parsowania wielomianu i wypisuje odpowiedni komunikat.
 * @return status wykonania dla błędu
 */
static bool ThrowParsePolyError()
{
    fprintf(stderr, "ERROR %d %d\n",
            global_pcalc_line_number, global_pcalc_column_number);
    return true;
}

/**
 * Zwraca błąd parsowania polecenia i wypisuje odpowiedni komunikat.
 * @return status wykonania dla błędu
 */
static bool ThrowParseCommandError()
{
    fprintf(stderr, "ERROR %d WRONG COMMAND\n", global_pcalc_line_number);
    return true;
}

/**
 * Zwraca błąd parsowania argumentu polecenia AT i wypisuje odpowiedni
 * komunikat.
 * @return status wykonania dla błędu
 */
static bool ThrowParseAtArgError()
{
    fprintf(stderr, "ERROR %d WRONG VALUE\n", global_pcalc_line_number);
    return true;
}

/**
 * Zwraca błąd parsowania argumentu polecenia DEG_BY i wypisuje odpowiedni
 * komunikat.
 * @return status wykonania dla błędu
 */
static bool ThrowParseDegByArgError()
{
    fprintf(stderr, "ERROR %d WRONG VARIABLE\n", global_pcalc_line_number);
    return true;
}

/**
 * Zwraca błąd parsowania argumentu polecenia COMPOSE i wypisuje odpowiedni
 * komunikat.
 * @return status wykonania dla błędu
 */
static bool ThrowParseComposeArgError()
{
    fprintf(stderr, "ERROR %d WRONG COUNT\n", global_pcalc_line_number);
    return true;
}

/**
 * Wykonuje na stosie wielomianów operację IS_ZERO.
 * Sprawdza, czy wielomian na wierzchołku stosu jest tożsamościowo równy zeru –
 * wypisuje na standardowe wyjście 0 lub 1.
 */
static void StackTopIsZero()
{
    PrintExpressionResult(PolyIsZero(GetStackTop(&global_pcalc_poly_stack)));
}

/**
 * Wykonuje na stosie wielomianów operację IS_COEFF.
 * Sprawdza, czy wielomian na wierzchołku stosu jest współczynnikiem –
 * wypisuje na standardowe wyjście 0 lub 1.
 */
static void StackTopIsCoeff()
{
    PrintExpressionResult(PolyIsCoeff(GetStackTop(&global_pcalc_poly_stack)));
}

/**
 * Wykonuje na stosie wielomianów operację DEG.
 * Wypisuje na standardowe wyjście stopień wielomianu
 * (−1 dla wielomianu tożsamościowo równego zeru).
 */
static void StackTopDeg()
{
    PrintExpressionResult(PolyDeg(GetStackTop(&global_pcalc_poly_stack)));
}

/**
 * Wykonuje na stosie wielomianów operację DEG_BY dla zadanego numeru zmiennej.
 * Wypisuje na standardowe wyjście stopień wielomianu ze względu na zmienną
 * o numerze idx (−1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] idx : indeks zmiennej
 */
static void StackTopDegBy(unsigned idx)
{
    PrintExpressionResult(PolyDegBy(GetStackTop(&global_pcalc_poly_stack), idx));
}

/**
 * Wykonuje na stosie wielomianów operację PRINT.
 * Wypisuje na standardowe wyjście wielomian z wierzchołka stosu w formacie
 * akceptowanym przez parser.
 */
static void StackTopPrint()
{
    PrintPoly(GetStackTop(&global_pcalc_poly_stack));
    printf("\n");
}

/**
 * Wykonuje na stosie wielomianów operację ZERO.
 * Wstawia na wierzchołek stosu wielomian tożsamościowo równy zeru.
 */
static void StackTopInsertZero()
{
    Poly *new_poly = PolyMalloc();
    *new_poly = PolyZero();
    PushOntoStack(new_poly, &global_pcalc_poly_stack);
}

/**
 * Wykonuje na stosie wielomianów operację CLONE.
 * Wstawia na stos kopię wielomianu z wierzchu stosu.
 */
static void StackTopClone()
{
    Poly *new_poly = PolyMalloc();
    *new_poly = PolyClone(GetStackTop(&global_pcalc_poly_stack));
    PushOntoStack(new_poly, &global_pcalc_poly_stack);
}

/**
 * Wykonuje na stosie wielomianów operację POP.
 * Usuwa wielomian z wierzchołka stosu.
 */
static void StackTopPop()
{
    Poly *a = global_pcalc_poly_stack.elem_pointer;
    PopStack(&global_pcalc_poly_stack);
    PolyDestroy(a);
    free(a);
}

/**
 * Wykonuje na stosie wielomianów operację NEG.
 * Neguje wielomian na wierzchołku stosu.
 */
static void StackTopNeg()
{
    Poly *a = PollStackTop(&global_pcalc_poly_stack);
    Poly *b = PolyMalloc();
    *b = PolyNeg(a);
    PolyDestroy(a);
    free(a);
    PushOntoStack(b, &global_pcalc_poly_stack);
}

/**
 * Wykonuje na stosie wielomianów operację IS_EQ.
 * Sprawdza, czy dwa wielomiany na wierzchu stosu są równe –
 * wypisuje na standardowe wyjście 0 lub 1.
 */
static void StackTopIsEq()
{
    Poly *a = PollStackTop(&global_pcalc_poly_stack);
    Poly *b = GetStackTop(&global_pcalc_poly_stack);
    PushOntoStack(a, &global_pcalc_poly_stack);
    PrintExpressionResult(PolyIsEq(a, b));
}

/**
 * Wykonuje na dwóch pierwszych elementach stosu daną operację i umieszcza jej
 * wynik na stosie.
 * @param operation : operacja do wykonania na stosie
 */
static void PushBinaryPolyOperationResultOntoStack
    (Poly (*Operation)(const Poly *a, const Poly *b))
{
    Poly *a = PollStackTop(&global_pcalc_poly_stack);
    Poly *b = PollStackTop(&global_pcalc_poly_stack);
    Poly *res = PolyMalloc();
    *res = Operation(a, b);
    PushOntoStack(res, &global_pcalc_poly_stack);
    PolyDestroy(a);
    free(a);
    PolyDestroy(b);
    free(b);
}

/**
 * Wykonuje na stosie wielomianów operację ADD.
 * Dodaje dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek
 * stosu ich sumę.
 */
static void StackTopAdd()
{
    PushBinaryPolyOperationResultOntoStack(PolyAdd);
}

/**
 * Wykonuje na stosie wielomianów operację MUL.
 * Mnoży dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek
 * stosu ich iloczyn.
 */
static void StackTopMul()
{
    PushBinaryPolyOperationResultOntoStack(PolyMul);
}

/**
 * Wykonuje na stosie wielomianów operację SUB.
 * Odejmuje od wielomianu z wierzchołka wielomian pod wierzchołkiem, usuwa je
 * i wstawia na wierzchołek stosu różnicę.
 */
static void StackTopSub()
{
    PushBinaryPolyOperationResultOntoStack(PolySub);
}

/**
 * Wykonuje na stosie wielomianów operację AT dla zadanej wartości zmiennej.
 * Wylicza wartość wielomianu w punkcie x, usuwa wielomian z wierzchołka i
 * wstawia na stos wynik operacji.
 * @param[in] x : wartość pierwszej ze zmiennych
 */
static void StackTopAt(poly_coeff_t x)
{
    Poly *a = PollStackTop(&global_pcalc_poly_stack);
    Poly *b = PolyMalloc();
    *b = PolyAt(a, x);
    PolyDestroy(a);
    free(a);
    PushOntoStack(b, &global_pcalc_poly_stack);
}

static void StackTopCompose(unsigned count)
{
    Poly *a = PollStackTop(&global_pcalc_poly_stack);
    Poly x[count];
    for (unsigned i = 0; i < count; ++i)
    {
        Poly *t = PollStackTop(&global_pcalc_poly_stack);
        x[i] = *t;
        free(t);
    }
    Poly *res = PolyMalloc();
    *res = PolyCompose(a, count, x);
    PushOntoStack(res, &global_pcalc_poly_stack);
    PolyDestroy(a);
    free(a);
    for (unsigned i = 0; i < count; ++i)
    {
        PolyDestroy(&x[i]);
    }
}


/**
 * Dodaje @p b do @p a.
 * Funkcja sprawdza, czy wykonanie takiego dodawania nie wykroczy poza
 * dany zakres, w którym to przypadku zwraca błędny status wykonania.
 * @param[in]  lower_limit : dolny zakres operacji
 * @param[in]  upper_limit : górny zakres operacji
 * @param[in,out]  a       : liczba, do której zostanie dodana wartość @p b
 * @param[in]  b           : wartość do dodania do @p a
 * @return             status wykonania
 */
static bool AddNumbers(long lower_limit, long upper_limit, long *a, long b)
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

/**
 * Mnoży @p a przez dziesięć.
 * Funkcja sprawdza, czy wykonanie takiego mnożenia nie wykroczy poza
 * dany zakres, w którym to przypadku zwraca błędny status wykonania.
 * @param[in]  lower_limit : dolny zakres operacji
 * @param[in]  upper_limit : górny zakres operacji
 * @param[in,out]  a       : liczba do przemnożenia przez 10
 * @return             status wykonania
 */
static bool MultiplyByTen(long lower_limit, long upper_limit, long *a)
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

/**
 * Parsuje liczbę ze standardowego wejścia.
 * Parser wczyta liczbę jedynie w przypadku, gdy liczba dostępna na wejściu
 * zawiera się w podanym zakresie.
 * Parser zwraca niezerowy status wykonania, gdy w trakcie jego działania
 * wystąpi błąd.
 * @param[in]  lower_limit : dolny zakres liczby do wczytania
 * @param[in]  upper_limit : górny zakres liczby do wczytania
 * @param[out] output      : wyjście parsera
 * @return             status wykonania parsowania
 */
static bool ParseNumber(long lower_limit, long upper_limit, long *output)
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

/**
 * Parsuje ze standardowego wejścia współczynnik.
 * Parser zwraca niezerowy status wykonania, gdy w trakcie jego działania
 * wystąpi błąd.
 * @param[out] out      : wyjście parsera
 * @return             status wykonania parsowania
 */
static bool ParseCoeff(poly_coeff_t *out)
{
    long parser_output;
    if (ParseNumber(LONG_MIN, LONG_MAX, &parser_output))
    {
        return true;
    }
    *out = parser_output;
    return false;
}

/**
 * Parsuje ze standardowego wejścia wartość wykładnika.
 * Parser zwraca niezerowy status wykonania, gdy w trakcie jego działania
 * wystąpi błąd.
 * @param[out] out      : wyjście parsera
 * @return             status wykonania parsowania
 */
static bool ParseExp(poly_exp_t *out)
{
    long parser_output;
    if (ParseNumber(0, INT_MAX, &parser_output))
    {
        return true;
    }
    *out = parser_output;
    return false;
}

/**
 * Wykonuje daną operację jedynie, gdy na stosie jest wystarczająca liczba elementów.
 * W przeciwnym wypadku zwraca błąd i wypisuje komunikat.
 * @param[in]  size_requirement : wymagana liczba elementów na stosie
 * @param[in]  procedure        : procedura do wykonania
 * @return                  status wykonania operacji na stosie
 */
static bool ExecuteOnPolyStack(unsigned size_requirement, void (*Procedure)())
{
    if (global_pcalc_poly_stack.size >= size_requirement)
    {
        Procedure();
        return false;
    }
    else
    {
        return ThrowStackUnderflow();
    }
}

static bool ParseArgument(long *out, long lower_limit, long upper_limit)
{
    if (!BufferIsNumber())
    {
        return true;
    }
    long arg;
    if (ParseNumber(lower_limit, upper_limit, &arg))
    {
        return true;
    }
    if (global_pcalc_read_buffer != '\n')
    {
        return true;
    }
    *out = arg;
    return false;
}

/**
 * Parsuje polecenie ze standardowego wejścia oraz wykonuje je.
 * Parser zwraca niezerowy status wykonania, gdy w trakcie jego działania
 * wystąpi błąd.
 * @return      status wykonania parsowania
 */
static bool ParseCommand()
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
            poly_coeff_t arg;
            if (ParseArgument(&arg, LONG_MIN, LONG_MAX))
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
            long arg;
            if (ParseArgument(&arg, 0, UINT_MAX))
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
        else if (strcmp(command, "COMPOSE") == 0)
        {
            long arg;
            if (ParseArgument(&arg, 0, INT_MAX))
            {
                return ThrowParseComposeArgError();
            }
            unsigned count = arg;
            if (global_pcalc_poly_stack.size > count)
            {
                StackTopCompose(count);
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

static bool ParsePoly(Poly *output);

/**
 * Parsuje ze standardowego wejścia jednomian.
 * Parser zwraca niezerowy status wykonania, gdy w trakcie jego działania
 * wystąpi błąd.
 * @param[out] output      : wyjście parsera
 * @return             status wykonania parsowania
 */
static bool ParseMono(Mono *output)
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

/**
 * Parsuje ze standardowego wejścia wielomian.
 * Parser zwraca niezerowy status wykonania, gdy w trakcie jego działania
 * wystąpi błąd.
 * @param[out] output      : wyjście parsera
 * @return             status wykonania parsowania
 */
static bool ParsePoly(Poly *output)
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

/**
 * Interpretuje jedną linię standardowego wejścia dla kalkulatora.
 * Założenie - po wykonaniu tego polecenia w buforze znajdować się będzie
 * ostatni znak zinterpretowanego właśnie wiersza.
 */
static void ParseLine()
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

/**
 * Główna funkcja kalkulatora wielomianów.
 * @return status wykonania
 */
int main()
{
    //inicjalizacja
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
