#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include "cmocka.h"
#include "poly.h"




int mock_fprintf(FILE*const file, const char *format, ...) CMOCKA_PRINTF_ATTRIBUTE(2, 3);
int mock_printf(const char *format, ...) CMOCKA_PRINTF_ATTRIBUTE(1, 2);

/**
 * Pomocnicze bufory, do których piszą atrapy funkcji printf i fprintf oraz
 * pozycje zapisu w tych buforach. Pozycja zapisu wskazuje bajt o wartości 0.
 */
static char fprintf_buffer[256];
static char printf_buffer[256];
static int fprintf_position = 0;
static int printf_position = 0;

/**
 * Atrapa funkcji fprintf sprawdzająca poprawność wypisywania na stderr.
 */
int mock_fprintf(FILE*const file, const char *format, ...)
{
    int return_value;
    va_list args;

    assert_true(file == stderr);
    /* Poniższa asercja sprawdza też, czy fprintf_position jest nieujemne.
       W buforze musi zmieścić się kończący bajt o wartości 0. */
    assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));

    va_start(args, format);
    return_value = vsnprintf(fprintf_buffer + fprintf_position,
                             sizeof(fprintf_buffer) - fprintf_position,
                             format,
                             args);
    va_end(args);

    fprintf_position += return_value;
    assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));
    return return_value;
}

/**
 * Atrapa funkcji fprintf sprawdzająca poprawność wypisywania na stdout.
 */
int mock_printf(const char *format, ...)
{
    int return_value;
    va_list args;

    /* Poniższa asercja sprawdza też, czy printf_position jest nieujemne.
       W buforze musi zmieścić się kończący bajt o wartości 0. */
    assert_true((size_t)printf_position < sizeof(printf_buffer));

    va_start(args, format);
    return_value = vsnprintf(printf_buffer + printf_position,
                             sizeof(printf_buffer) - printf_position,
                             format,
                             args);
    va_end(args);

    printf_position += return_value;
    assert_true((size_t)printf_position < sizeof(printf_buffer));
    return return_value;
}

/**
 *  Pomocniczy bufor, z którego korzystają atrapy funkcji operujących na stdin.
 */
static char input_stream_buffer[256];
static int input_stream_position = 0;
static int input_stream_end = 0;
int read_char_count;

/**
 * Atrapa funkcji scanf używana do przechwycenia czytania z stdin.
 */
int mock_scanf(const char *format, ...)
{
    va_list fmt_args;
    int ret;

    va_start(fmt_args, format);
    ret = vsscanf(input_stream_buffer + input_stream_position, format, fmt_args);
    va_end(fmt_args);

    if (ret < 0)   /* ret == EOF */
    {
        input_stream_position = input_stream_end;
    }
    else
    {
        assert_true(read_char_count >= 0);
        input_stream_position += read_char_count;
        if (input_stream_position > input_stream_end)
        {
            input_stream_position = input_stream_end;
        }
    }
    return ret;
}

/**
 * Atrapa funkcji getchar używana do przechwycenia czytania z stdin.
 */
int mock_getchar()
{
    if (input_stream_position < input_stream_end)
    {
        return input_stream_buffer[input_stream_position++];
    }
    else
    {
        return EOF;
    }
}

/**
 * Atrapa funkcji ungetc.
 * Obsługiwane jest tylko standardowe wejście.
 */
int mock_ungetc(int c, FILE *stream)
{
    assert_true(stream == stdin);
    if (input_stream_position > 0)
    {
        return input_stream_buffer[--input_stream_position] = c;
    }
    else
    {
        return EOF;
    }
}


static void ZeroPolyZeroComposeTest(void **state)
{
    (void)state;

    Poly zero_poly = PolyZero();
    Poly result = PolyCompose(&zero_poly, 0, NULL);

    assert_int_equal(result.abs_term, zero_poly.abs_term);
    assert_int_equal(result.abs_term, 0);
    assert_int_equal(result.first, NULL);
    assert_int_equal(result.last, NULL);
}

static void ZeroPolyCoeffComposeTest(void **state)
{
    (void)state;

    poly_coeff_t coeff = rand();

    Poly zero_poly = PolyZero();
    Poly arg = PolyFromCoeff(coeff);
    Poly result = PolyCompose(&zero_poly, 1, &arg);

    assert_int_equal(result.abs_term, zero_poly.abs_term);
    assert_int_equal(result.abs_term, 0);
    assert_int_equal(result.first, NULL);
    assert_int_equal(result.last, NULL);
}

static void CoeffPolyZeroComposeTest(void **state)
{
    (void)state;

    poly_coeff_t coeff = rand();

    Poly coeff_poly = PolyFromCoeff(coeff);
    Poly result = PolyCompose(&coeff_poly, 0, NULL);

    assert_int_equal(result.abs_term, coeff_poly.abs_term);
    assert_int_equal(result.abs_term, coeff);
    assert_int_equal(result.first, NULL);
    assert_int_equal(result.last, NULL);
}

static void CoeffPolyCoeffComposeTest(void **state)
{
    (void)state;

    poly_coeff_t coeff_1 = rand();
    poly_coeff_t coeff_2 = rand();

    Poly coeff_poly_1 = PolyFromCoeff(coeff_1);
    Poly coeff_poly_2 = PolyFromCoeff(coeff_2);
    Poly result = PolyCompose(&coeff_poly_1, 1, &coeff_poly_2);

    assert_int_equal(result.abs_term, coeff_poly_1.abs_term);
    assert_int_equal(result.abs_term, coeff_1);
    assert_int_equal(result.first, NULL);
    assert_int_equal(result.last, NULL);
}

int main(void)
{
    srand(time(NULL));
    const struct CMUnitTest poly_compose_tests[] = {
        cmocka_unit_test(ZeroPolyZeroComposeTest),
        cmocka_unit_test(ZeroPolyCoeffComposeTest),
        cmocka_unit_test(CoeffPolyZeroComposeTest),
        cmocka_unit_test(CoeffPolyCoeffComposeTest),
    };
    return cmocka_run_group_tests(poly_compose_tests, NULL, NULL);
}
