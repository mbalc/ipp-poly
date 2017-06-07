#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <setjmp.h>
#include "cmocka.h"
#include "poly.h"


static jmp_buf jmp_at_exit;
static int exit_status;

extern int calc_poly_main();

int mock_main()
{
    if (!setjmp(jmp_at_exit))
    {
        return calc_poly_main();
    }
    return exit_status;
}

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

static Poly poly_arg_1;
static Poly poly_arg_2;
static Poly result;

static Poly expected;


/**
 * Funkcja wołana przed każdym testem..
 */
static int pc_test_setup(void **state)
{
    (void)state;

    /* Zwrócenie zera oznacza sukces. */
    return 0;
}

/**
 * Funkcja wołana po każdym teście.
 */
static int pc_test_teardown(void **state)
{
    (void)state;

    assert_int_equal(result.abs_term, expected.abs_term);
    if (expected.first == NULL)
    {
        assert_int_equal(result.first, NULL);
        assert_int_equal(result.last, NULL);
    }
    else
    {
        printf("linear on linear only \n");
    }

    /* Zwrócenie zera oznacza sukces. */
    return 0;
}


static void ZeroPolyNullComposeTest(void **state)
{
    (void)state;

    poly_arg_1 = PolyZero();
    poly_arg_2 = PolyZero();
    result = PolyCompose(&poly_arg_1, 0, NULL);
    expected = PolyZero();
}

static void ZeroPolyCoeffComposeTest(void **state)
{
    (void)state;

    poly_coeff_t coeff = rand();

    poly_arg_1 = PolyZero();
    poly_arg_2 = PolyFromCoeff(coeff);
    result = PolyCompose(&poly_arg_1, 1, &poly_arg_2);
    expected = PolyZero();
}

static void CoeffPolyNullComposeTest(void **state)
{
    (void)state;

    poly_coeff_t coeff = rand();

    poly_arg_1 = PolyFromCoeff(coeff);
    poly_arg_2 = PolyZero();
    result = PolyCompose(&poly_arg_1, 0, NULL);
    expected = PolyFromCoeff(coeff);
}

static void CoeffPolyCoeffComposeTest(void **state)
{
    (void)state;

    poly_coeff_t coeff_1 = rand();
    poly_coeff_t coeff_2;
    do
    {
        coeff_2 = rand();
    } while (coeff_2 == coeff_1);

    poly_arg_1 = PolyFromCoeff(coeff_1);
    poly_arg_2 = PolyFromCoeff(coeff_2);
    result = PolyCompose(&poly_arg_1, 1, &poly_arg_2);
    expected = PolyFromCoeff(coeff_1);
}

static void LinearPolyNullComposeTest(void **state)
{
    (void)state;
    Poly cf;
    Mono linear_mono;

    cf = PolyFromCoeff(1);
    linear_mono = MonoFromPoly(&cf, 1);

    poly_arg_1 = PolyAddMonos(1, &linear_mono);
    poly_arg_2 = PolyZero();
    result = PolyCompose(&poly_arg_1, 0, NULL);
    expected = PolyZero();

    PolyDestroy(&result);
    PolyDestroy(&poly_arg_1);
}

static void LinearPolyCoeffComposeTest(void **state)
{
    (void)state;
    Poly cf;
    Mono linear_mono;

    cf = PolyFromCoeff(1);
    linear_mono = MonoFromPoly(&cf, 1);

    poly_coeff_t coeff = rand();


    poly_arg_1 = PolyAddMonos(1, &linear_mono);
    poly_arg_2 = PolyFromCoeff(coeff);
    result = PolyCompose(&poly_arg_1, 1, &poly_arg_2);
    expected = PolyFromCoeff(coeff);

    PolyDestroy(&result);
    PolyDestroy(&poly_arg_1);
}

static void LinearPolyLinearComposeTest(void **state)
{
    (void)state;
    Poly cf;
    Mono linear_mono;

    cf = PolyFromCoeff(1);
    linear_mono = MonoFromPoly(&cf, 1);

    poly_arg_1 = PolyAddMonos(1, &linear_mono);
    poly_arg_2 = PolyClone(&poly_arg_1);
    result = PolyCompose(&poly_arg_1, 1, &poly_arg_2);
    expected = PolyAddMonos(1, &linear_mono);

    assert_true(PolyIsCoeff(&result.first->p));
    assert_int_equal(result.first->p.abs_term, 1);
    assert_int_equal(result.first->exp, expected.first->exp);

    PolyDestroy(&result);
    PolyDestroy(&expected);
    PolyDestroy(&poly_arg_1);
    PolyDestroy(&poly_arg_2);
}






static void init_input_stream(const char *str)
{
    memset(input_stream_buffer, 0, sizeof(input_stream_buffer));
    input_stream_position = 0;
    input_stream_end = strlen(str);
    assert_true((size_t)input_stream_end < sizeof(input_stream_buffer));
    strcpy(input_stream_buffer, str);
}

static int count_test_setup(void **state)
{
    (void)state;

    memset(fprintf_buffer, 0, sizeof(fprintf_buffer));
    memset(printf_buffer, 0, sizeof(printf_buffer));
    printf_position = 0;
    fprintf_position = 0;

    /* Zwrócenie zera oznacza sukces. */
    return 0;
}

static void NullCountArgTest(void **state)
{
    (void)state;

    init_input_stream("COMPOSE ");
    mock_main();
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

static void ZeroCountArgTest(void **state)
{
    (void)state;

    init_input_stream("(1,2)\nCOMPOSE 0");
    mock_main();
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "");
}

static void UINT_MAXCountArgTest(void **state)
{
    (void)state;

    char input [256];
    sprintf(input, "(1,2)\nCOMPOSE %ld", (long)UINT_MAX);  //4294967295

    init_input_stream(input);

    mock_main();
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 2 STACK UNDERFLOW\n");
}

static void NegativeCountArgTest(void **state)
{
    (void)state;

    init_input_stream("COMPOSE -1");

    mock_main();
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

static void UINT_MAXPlusOneCountArgTest(void **state)
{
    (void)state;

    char input [256];
    sprintf(input, "(1,2)\nCOMPOSE %ld", (long)UINT_MAX + 1); //4294967296

    init_input_stream(input);

    mock_main();
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 2 WRONG COUNT\n");
}

static void VeryBigCountArgTest(void **state)
{
    (void)state;

    char input [256];
    sprintf(input, "(1,2)\nCOMPOSE %ld", LONG_MAX - UINT_MAX); //9223372032559808512

    init_input_stream(input);

    mock_main();
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 2 WRONG COUNT\n");
}

static void RandomLettersCountArgTest(void **state)
{
    (void)state;

    init_input_stream("COMPOSE rAbTVSpPCHjhLAI");

    mock_main();
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

static void RandomLettersAndNumbersCountArgTest(void **state)
{
    (void)state;

    init_input_stream("COMPOSE 44Kpv6hAdaWK0HF");

    mock_main();
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}


int main(void)
{
    srand(time(NULL));
    const struct CMUnitTest poly_compose_tests[] = {
        cmocka_unit_test_setup_teardown(ZeroPolyNullComposeTest, pc_test_setup, pc_test_teardown),
        cmocka_unit_test_setup_teardown(ZeroPolyCoeffComposeTest, pc_test_setup, pc_test_teardown),
        cmocka_unit_test_setup_teardown(CoeffPolyNullComposeTest, pc_test_setup, pc_test_teardown),
        cmocka_unit_test_setup_teardown(CoeffPolyCoeffComposeTest, pc_test_setup, pc_test_teardown),
        cmocka_unit_test_setup_teardown(LinearPolyNullComposeTest, pc_test_setup, pc_test_teardown),
        cmocka_unit_test_setup_teardown(LinearPolyCoeffComposeTest, pc_test_setup, pc_test_teardown),
        cmocka_unit_test_setup_teardown(LinearPolyLinearComposeTest, pc_test_setup, pc_test_teardown),
    };
    const struct CMUnitTest count_calc_tests[] = {
        cmocka_unit_test_setup(NullCountArgTest, count_test_setup),
        cmocka_unit_test_setup(ZeroCountArgTest, count_test_setup),
        cmocka_unit_test_setup(UINT_MAXCountArgTest, count_test_setup),
        cmocka_unit_test_setup(NegativeCountArgTest, count_test_setup),
        cmocka_unit_test_setup(UINT_MAXPlusOneCountArgTest, count_test_setup),
        cmocka_unit_test_setup(VeryBigCountArgTest, count_test_setup),
        cmocka_unit_test_setup(RandomLettersCountArgTest, count_test_setup),
        cmocka_unit_test_setup(RandomLettersAndNumbersCountArgTest, count_test_setup),
    };

    bool status = 0;

    status |= cmocka_run_group_tests(poly_compose_tests, NULL, NULL);
    status |= cmocka_run_group_tests(count_calc_tests, NULL, NULL);

    return status;
}
