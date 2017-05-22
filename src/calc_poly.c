#include <stdio.h>
#include "stack.h"

char global_read_buffer;


void ParseLine()
{
    global_read_buffer = getchar();
    if (('a' <= global_read_buffer && global_read_buffer <= 'z') ||
        ('A' <= global_read_buffer && global_read_buffer <= 'Z'))
    {

    }
}
