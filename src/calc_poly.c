#include <stdio.h>
#include "poly.h"
#include "stack.h"

char global_pcalc_read_buffer;
PointerStack global_pcalc_poly_stack;

unsigned global_pcalc_line_number;
unsigned global_pcalc_column_number;

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


int ParseInt()
{
    int out = 0;
    bool negative = (global_pcalc_read_buffer == '-');
    if (negative)
    {
        ReadCharacter();
    }
    while ('0' <= global_pcalc_read_buffer && global_pcalc_read_buffer <= '9')
    {
        out *= 10;
        out += (negative ? -1 : 1) * (global_pcalc_read_buffer - '0');
        ReadCharacter();
    }
    return out;
}

bool ParseCommand()
{
    return false; //not implemented yet
}

bool ParsePoly()
{
    printf("%d\n", ParseInt());
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
