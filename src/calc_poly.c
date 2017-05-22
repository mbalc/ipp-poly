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

bool ParseCommand()
{
    return false;
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
            while (global_pcalc_read_buffer != '\n'
                   && global_pcalc_read_buffer != 0)
            {
                ReadCharacter();
            }
        }
    }
    else
    {
        printf("Blasfd");
    }
}

int main()
{
    global_pcalc_read_buffer = 1;
    while (global_pcalc_read_buffer != 0)
        ParseLine();
    return 0;
}
