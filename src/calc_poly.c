#include <stdio.h>
#include <limits.h>
#include "poly.h"
#include "stack.h"

char global_pcalc_read_buffer;
PointerStack global_pcalc_poly_stack;

unsigned global_pcalc_line_number;
unsigned global_pcalc_column_number;

bool BufferIsNumber()
{
    return '0' <= global_pcalc_read_buffer && global_pcalc_read_buffer <= '9';
}

void ReadCharacter()
{
    if (global_pcalc_read_buffer == '\n')
    {
        global_pcalc_line_number += 1;
        global_pcalc_column_number = 1;
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
    global_pcalc_line_number += 1;
    global_pcalc_column_number = 1;
}

bool AddNumbers(long lower_limit, long upper_limit, long *a, long b)
{
    printf("tryina add %ld to %ld\n", *a, b);
    if (upper_limit - *a < b)
    {
        return false;
    }
    if (*a - lower_limit < -b)
    {
        return false;
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
    long *out = (long*)output;
    *out = 0;
    bool negative = (global_pcalc_read_buffer == '-');
    if (negative)
    {
        ReadCharacter();
    }
    while (BufferIsNumber())
    {
        if (!MultiplyByTen(lower_limit, upper_limit, out))
        {
            return false;
        }
        if (!AddNumbers(lower_limit, upper_limit, out,
                        (negative ? -1 : 1) * (global_pcalc_read_buffer - '0')))
        {
            return false;
        }
        ReadCharacter();
    }
    return true;
}

bool ParseCommand()
{
    return false; //not implemented yet
}

bool ParsePoly(Poly *output)
{
    if (global_pcalc_read_buffer == '(')
    {
        ReadCharacter();
        ParseMono();
    }
    printf("%ld\n", ParseNumber(0, LONG_MAX));
    return false; //not implemented yet
}


void ParseLine()
{
    ReadCharacter();
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
        if (!ParsePoly())
        {
            printf("PolyParseError\n");
            ReadUntilNewline();
        }
    }
}

int main()
{
    global_pcalc_read_buffer = 1;
    while (global_pcalc_read_buffer != 0)
        ParseLine();
    return 0;
}
