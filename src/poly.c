/** @file
   Implementacja interfejsu klasy wielomianów

   @author Michał Balcerzak <mb385130@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-13
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "poly.h"


/**
 * Wypisuje wielomian, gdy nazwa jego głównego parametru to @p c.
 * Wypisuje zawartość struktury danego wielomianu na standardowe wyjście.
 * Kolejne zmienne wielomianu sa nazywane kolejnymi literami alfabetu od @p c
 * począwszy.
 * @param[in] p : wielomian
 * @param[in] c : literowa nazwa parametru
 */
static void PrintPolyList(const Poly *p, char c)
{
    if (p->abs_term > 0)
    {
        printf("%ld", p->abs_term);
    }
    if (p->abs_term < 0)
    {
        printf("(%ld)", p->abs_term);
    }
    for (Mono *ptr = p->last; ptr != NULL; ptr = ptr->prev)
    {
        if (ptr != p->last || p->abs_term != 0)
        {
            printf("+");
        }
        if (PolyIsCoeff(&ptr->p))
        {
            PrintPolyList(&ptr->p, c + 1);
        }
        else
        {
            printf("(");
            PrintPolyList(&ptr->p, c + 1);
            printf(")");
        }
        printf("%c^%ld", c, ptr->exp);
    }
}

/**
 * @details Implementacja procedury PrintPoly udokumentowanej w pliku poly.h.
 * Kolejne zmienne wielomianu sa nazywane kolejnymi literami alfabetu
 * łacińskiego.
 * @param[in] p : wielomian
 */
void PrintPoly(const Poly *p)
{
    PrintPolyList(p, 'a');
}

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
 * Łączy jednomiany @p left i @p right tak, by sąsiadowały ze sobą.
 * Modyfikuje wskaźniki na sąsiadów w danych jednomianach.
 * @param[in, out] left : jednomian - lewy sąsiad @p right
 * @param[in, out] right : jednomian - prawy sąsiad @p left
 */
static void LinkMonos(Mono *left, Mono *right)
{
    if (left != NULL)
    {
        left->next = right;
    }
    if (right != NULL)
    {
        right->prev = left;
    }
}

/**
 * Usuwa z wielomianu jednomian o największym wykładniku i niszczy go.
 * Przepina wskaźniki w danym wielomianie tak, że ten pozostaje zgodny
 * z dokumentacją.
 * @param[in, out] p : wielomian
 */
static void PolyTruncate(Poly *p)
{
    Mono *m = p->first;
    p->first = m->next;
    if (m->next == NULL)
    {
        p->last = NULL;
    }
    LinkMonos(NULL, m->next);
    MonoDestroy(m);
}

/**
 * @details Implementacja procedury PolyDestroy udokumentowanej w pliku poly.h.
 * @param[in] p : wielomian
 */
void PolyDestroy(Poly *p)
{
    for (Mono *ptr = p->first; ptr != NULL; ptr = p->first)
    {
        PolyTruncate(p);
        free(ptr);
    }
}

/**
 * Dodaje jednomian @p val na początek wielomianu @p p.
 * @param[in, out] p   : wielomian
 * @param[in] val      : jednomian do wstawienia
 */
static void PolyAppendMono(Poly *p, Mono val)
{
    Mono *m = MonoMalloc();
    *m = val;
    LinkMonos(m, p->first);
    LinkMonos(NULL, m);
    p->first = m;
    if (p->last == NULL)
    {
        p->last = m;
    }
}

/**
 * @details Implementacja procedury PolyClone udokumentowanej w pliku poly.h.
 * @param[in] p : wielomian
 */
Poly PolyClone(const Poly *p)
{
    Poly out = PolyFromCoeff(p->abs_term);
    for (Mono *ptr = p->last; ptr != NULL; ptr = ptr->prev)
    {
        PolyAppendMono(&out, MonoClone(ptr));
    }
    return out;
}

/**
 * @details Implementacja procedury PolyAdd udokumentowanej w pliku poly.h.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p + q`
 */
Poly PolyAdd(const Poly *p, const Poly *q)
{
    Poly out = PolyFromCoeff(p->abs_term + q->abs_term);
    Mono *p_ptr = p->last, *q_ptr = q->last;
    Mono buf;
    while (p_ptr != NULL && q_ptr != NULL)
    {
        if (p_ptr->exp == q_ptr->exp)
        {
            buf.exp = p_ptr->exp;
            buf.p = PolyAdd(&p_ptr->p, &q_ptr->p);
            p_ptr = p_ptr->prev;
            q_ptr = q_ptr->prev;
        }
        else if (p_ptr->exp < q_ptr->exp)
        {
            buf = MonoClone(p_ptr);
            p_ptr = p_ptr->prev;
        }
        else if (p_ptr->exp > q_ptr->exp)
        {
            buf = MonoClone(q_ptr);
            q_ptr = q_ptr->prev;
        }
        if (!PolyIsZero(&buf.p))
        {
            PolyAppendMono(&out, buf);
        }
    }
//jednomiany z wielomianu o większym wykładniku głównej zmiennej
    if (q_ptr != NULL)
    {
        p_ptr = q_ptr;
    }
    while (p_ptr != NULL)
    {
        PolyAppendMono(&out, MonoClone(p_ptr));
        p_ptr = p_ptr->prev;
    }
    return out;
}

/**
 * Porównuje wykładniki dwóch jednomianów.
 * Procedura wykorzystywana do posortowania tablicy jednomianów w PolyAddMonos.
 * @param[in] a : pierwszy jednomian
 * @param[in] b : drugi jednomian
 * @return    0 gdy jednomiany są ustawione w kolejności malejącej względem
 * wykładników \n
 * 1 w przeciwnym wypadku
 */
static int CompareMonos(const void *a, const void *b)
{
    return ((Mono*)a)->exp < ((Mono*)b)->exp;
}

/**
 * @details Implementacja procedury PolyAddMonos udokumentowanej w pliku poly.h.
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyAddMonos(unsigned count, const Mono monos[])
{
//kopia tablicy wskaźników
    Mono *arr = calloc(count, sizeof(Mono));
    for (unsigned i = 0; i < count; ++i)
    {
        arr[i] = monos[i];
    }
//ustawiam tablicę arr[] w kolejności malejącej względem wykładników
    qsort(arr, count, sizeof(Mono), CompareMonos);
    Poly out = PolyZero(), aux;
    Mono buf = arr[count - 1];
    for (int i = count - 2; i >= 0; --i)
    {
        if (arr[i].exp == buf.exp)
        {

            aux = PolyAdd(&buf.p, &arr[i].p);
            PolyDestroy(&buf.p);
            PolyDestroy(&arr[i].p);
            buf.p = aux;
        }

        if (arr[i].exp > buf.exp)
        {
            if (!PolyIsZero(&buf.p))
            {
                PolyAppendMono(&out, buf);
            }
            buf = arr[i];
        }
        if (arr[i].exp < buf.exp)
        {
            assert(false);
        }
    }
 
    PolyAppendMono(&out, buf);
    free(arr);
  

    if (out.last->exp == 0)
    {
        out.abs_term = out.last->p.abs_term;
        out.last->p.abs_term = 0;
    }
//gdy właśnie wyzerowaliśmy najmniejszy z jednomianów
    if (PolyIsZero(&out.last->p))
    {
        Mono *ptr = out.last->prev;
        MonoDestroy(out.last);
        free(out.last);
        out.last = ptr;
        if (out.last == NULL)
        {
            out.first = NULL;
        }
        LinkMonos(out.last, NULL);
    }
    
   return out;
}

/**
 * @details Implementacja procedury PolyCoeffMul udokumentowanej w pliku poly.h.
 * @param[in]  p : wielomian
 * @param[in]  x : stała
 * @return 'p * x'
 */
Poly PolyCoeffMul(const Poly *p, poly_coeff_t x)
{
    Poly out = PolyFromCoeff(p->abs_term * x);
    Mono buf;
    for (Mono *p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
    {
        buf.exp = p_ptr->exp;
        buf.p = PolyCoeffMul(&p_ptr->p, x);
        if (PolyIsZero(&buf.p))
        {
            PolyDestroy(&buf.p);
        }
        else
        {
            PolyAppendMono(&out, buf);
        }
    }
    return out;
}

/**
 * @details Implementacja procedury PolyMul udokumentowanej w pliku poly.h.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p * q`
 */
Poly PolyMul(const Poly *p, const Poly *q)
{
    Poly aux = PolyCoeffMul(q, p->abs_term);
    Poly buffer = PolyCoeffMul(p, q->abs_term);
    Poly out = PolyAdd(&aux, &buffer);
    out.abs_term /= 2;
    PolyDestroy(&aux);
    PolyDestroy(&buffer);
    Mono buf;
    for (Mono *p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
    {
        buffer = PolyZero();
        for (Mono *q_ptr = q->last; q_ptr != NULL; q_ptr = q_ptr->prev)
        {
            buf.exp = p_ptr->exp + q_ptr->exp;
            buf.p = PolyMul(&p_ptr->p, &q_ptr->p);
            PolyAppendMono(&buffer, buf);
        }
        aux = PolyAdd(&out, &buffer);
        PolyDestroy(&out);
        out = aux;
        PolyDestroy(&buffer);
    }
    return out;
}

/**
 * @details Implementacja procedury PolyNeg udokumentowanej w pliku poly.h.
 * @param[in] p : wielomian
 * @return `-p`
 */
Poly PolyNeg(const Poly *p)
{
    Poly out = PolyFromCoeff(-p->abs_term);
    Mono buf;
    for (Mono *ptr = p->last; ptr != NULL; ptr = ptr->prev)
    {
        buf.p = PolyNeg(&ptr->p);
        buf.exp = ptr->exp;
        PolyAppendMono(&out, buf);
    }
    return out;
}

/**
 * @details Implementacja procedury PolySub udokumentowanej w pliku poly.h.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p - q`
 */
Poly PolySub(const Poly *p, const Poly *q)
{
    Poly neg = PolyNeg(q);
    Poly out = PolyAdd(p, &neg);
    PolyDestroy(&neg);
    return out;
}

/**
 * Zwraca większy z danych wykładników.
 * @param[in]  a : wykładnik
 * @param[in]  b : wykładnik
 * @return 'max(a, b)'
 */
static poly_exp_t MaxExp(poly_exp_t a, poly_exp_t b)
{
    if (a < b)
    {
        return b;
    }
    return a;
}

/**
 * Zwraca  - zależnie od parametrów - stopień wielomianu ze względu na zadaną
 * zmienną albo stopień całego wielomianu. Dla ujemnych wartości parametru
 * @p var_idx oblicza stopień całego wielomianu wedle specyfikacji PolyDeg.
 * W przeciwnym przypadku oblicza stopień wielomianu dla n-tej z kolei zmiennej
 * wielomianu (dla n = var_idx + 1) według specyfikacji PolyDegBy.
 * @param[in] p       : wielomian
 * @param[in] var_idx : parametr pomocniczy określający działanie procedury
 * @return stopień wielomianu - dla ujemnych @p var_idx \n
 * stopień wielomianu ze względu na zmienną o numerze @p var_idx - w przeciwnym
 * przypadku
 */
static poly_exp_t PolyDegEvaluate(const Poly *p, long var_idx)
{
    if (PolyIsZero(p))
    {
        return -1;
    }
    if (PolyIsCoeff(p))
    {
        return 0;
    }
    if (var_idx == 0)
    {
        return p->first->exp;
    }
    poly_exp_t out = 0;
    for (Mono *ptr = p->last; ptr != NULL; ptr = ptr->prev)
    {
        if (var_idx < 0)
        {
            out = MaxExp(out, PolyDegEvaluate(&ptr->p, var_idx) + ptr->exp);
        }
        else
        {
            out = MaxExp(out, PolyDegEvaluate(&ptr->p, var_idx - 1));
        }
    }
    return out;
}

/**
 * @details Implementacja procedury PolyDegBy udokumentowanej w pliku poly.h.
 * Zmienne indeksowane są od 0.
 * Zmienna o indeksie 0 oznacza zmienną główną tego wielomianu.
 * Większe indeksy oznaczają zmienne wielomianów znajdujących się
 * we współczynnikach.
 * @param[in] p : wielomian
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx
 */
poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx)
{
    return (PolyDegEvaluate(p, var_idx));
}

///Ujemna liczba podawana przez PolyDeg do PolyDegEvaluate
///jako alternatywne @p var_idx w celu zliczenia stopnia całego wielomianu
static const int EVALUATE_DEG = -1;

/**
 * @details Implementacja procedury PolyDeg udokumentowanej w pliku poly.h.
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDeg(const Poly *p)
{
    return PolyDegEvaluate(p, EVALUATE_DEG);
}

/**
 * Sprawdza równość dwóch jednomianów.
 * @param[in] a : jednomian
 * @param[in] b : jednomian
 * @return `a = b`
 */
static bool MonoIsEq(const Mono *a, const Mono *b)
{
    if (a->exp != b->exp)
    {
        return false;
    }
    return PolyIsEq(&a->p, &b->p);
}

/**
 * @details Implementacja procedury PolyIsEq udokumentowanej w pliku poly.h.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p = q`
 */
bool PolyIsEq(const Poly *p, const Poly *q)
{
    Mono *p_ptr = p->last, *q_ptr = q->last;
    while (p_ptr != NULL && q_ptr != NULL)
    {
        if (!MonoIsEq(p_ptr, q_ptr))
        {
            return false;
        }
        p_ptr = p_ptr->prev;
        q_ptr = q_ptr->prev;
    }
    if (p_ptr != q_ptr)
    {
        return false;
    }
    return p->abs_term == q->abs_term;
}

/**
 * Oblicza w logarytmicznym czasie wartość liczby @f$x^e@f$.
 * @param[in]  x : liczba do spotęgowania
 * @param[in]  e : wykładnik docelowej potęgi
 * @return @f$x^e@f$
 */
static poly_coeff_t FastPower(poly_coeff_t x, poly_exp_t e)
{
    if (e == 0)
    {
        return 1;
    }
    poly_coeff_t m;
    if (e % 2 == 0)
    {
        m = FastPower(x, e / 2);
        return m * m;
    }
    else
    {
        return x * FastPower(x, e - 1);
    }
}

/**
 * @details Implementacja procedury PolyAt udokumentowanej w pliku poly.h.
 * Wstawia pod pierwszą zmienną wielomianu wartość @p x.
 * W wyniku może powstać wielomian, jeśli współczynniki są wielomianem
 * i zmniejszane są indeksy zmiennych w takim wielomianie o jeden.
 * Formalnie dla wielomianu @f$p(x_0, x_1, x_2, \ldots)@f$ wynikiem jest
 * wielomian @f$p(x, x_0, x_1, \ldots)@f$.
 * @param[in] p : wielomian
 * @param[in] x : wartość pierwszej ze zmiennych
 * @return @f$p(x, x_0, x_1, \ldots)@f$
 */
Poly PolyAt(const Poly *p, poly_coeff_t x)
{
    Poly out = PolyFromCoeff(p->abs_term);
    Poly buffer, aux;
    poly_coeff_t a = 1, e = 0;
    for (Mono *ptr = p->last; ptr != NULL; ptr = ptr->prev)
    {
        buffer = PolyCoeffMul(&ptr->p, a * FastPower(x, ptr->exp - e));
        aux = PolyAdd(&buffer, &out);
        PolyDestroy(&buffer);
        PolyDestroy(&out);
        out = aux;
    }
    return out;
}
