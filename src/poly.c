#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "poly.h"


void PrintPoly(const Poly *p, char x)
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
            PrintPoly(&ptr->p, x + 1);
        }
        else
        {
            printf("(");
            PrintPoly(&ptr->p, x + 1);
            printf(")");
        }
        printf("%c^%ld", x, ptr->exp);
    }
}

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

void RemoveMonoFromPoly(Poly *p, Mono *m)
{
    p->first = m->next;
    if (m->next == NULL)
    {
        p->last = NULL;
    }
    LinkMonos(NULL, m->next);
    MonoDestroy(m);
}

void PolyDestroy(Poly *p)
{
    for (Mono *ptr = p->first; ptr != NULL; ptr = p->first)
    {
        RemoveMonoFromPoly(p, ptr);
        free(ptr);
    }
}

void ExtendPoly(Poly *p, Mono val)
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

Poly PolyClone(const Poly *p)
{
    Poly out = PolyFromCoeff(p->abs_term);
    for (Mono *ptr = p->last; ptr != NULL; ptr = ptr->prev)
    {
        ExtendPoly(&out, MonoClone(ptr));
    }
    return out;
}

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
            ExtendPoly(&out, buf);
        }
    }

    if (q_ptr != NULL)
    {
        p_ptr = q_ptr;
    }
    while (p_ptr != NULL)
    {
        ExtendPoly(&out, MonoClone(p_ptr));
        p_ptr = p_ptr->prev;
    }
    return out;
}

int CompareMonos(const void *a, const void *b)
{
    return ((Mono*)a)->exp < ((Mono*)b)->exp;
}

Poly PolyAddMonos(unsigned count, const Mono monos[])
{
    assert(count > 0);
    Mono *arr = calloc(count, sizeof(Mono));
    arr = memcpy(arr, monos, count * sizeof(Mono));
    qsort(arr, count, sizeof(Mono), CompareMonos);
    Poly out = PolyZero();
    Mono buf = arr[count - 1];
    for (int i = count - 2; i >= 0; --i)
    {
        if (arr[i].exp == buf.exp)
        {
            buf.p = PolyAdd(&buf.p, &arr[i].p);
        }

        if (arr[i].exp > buf.exp)
        {
            if (!PolyIsZero(&buf.p))
            {
                ExtendPoly(&out, buf);
            }
            buf = arr[i];
        }
        if (arr[i].exp < buf.exp)
        {
            assert(false);
        }
    }
    ExtendPoly(&out, buf);
    free(arr);
    if (out.last->exp == 0)
    {
        out.abs_term = out.last->p.abs_term;
        out.last->p.abs_term = 0;
        if (PolyIsZero(&out.last->p))
        {
            Mono *ptr = out.last->prev;
            MonoDestroy(out.last);
            if (ptr != NULL)
            {
                free(out.last);
            }
            out.last = ptr;
            LinkMonos(out.last, NULL);
        }
    }
    return out;
}

Poly PolyCoeffMul(const Poly *p, poly_coeff_t x)
{
    Poly out = PolyFromCoeff(p->abs_term * x);
    Mono buf;
    for (Mono *p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
    {
        buf.exp = p_ptr->exp;
        buf.p = PolyCoeffMul(&p_ptr->p, x);
        ExtendPoly(&out, buf);
    }
    return out;
}

Poly PolyMul(const Poly *p, const Poly *q)
{
    Poly out = PolyCoeffMul(q, p->abs_term);
    Poly buffer = PolyCoeffMul(p, q->abs_term);
    out = PolyAdd(&out, &buffer);
    out.abs_term /= 2;

    Mono buf;
    for (Mono *p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
    {
        buffer = PolyZero();
        for (Mono *q_ptr = q->last; q_ptr != NULL; q_ptr = q_ptr->prev)
        {
            buf.exp = p_ptr->exp + q_ptr->exp;
            buf.p = PolyMul(&p_ptr->p, &q_ptr->p);
            ExtendPoly(&buffer, buf);
        }
        out = PolyAdd(&out, &buffer);
    }
    return out;
}

Poly PolyNeg(const Poly *p)
{
    Poly out = PolyFromCoeff(-p->abs_term);
    Poly mem;
    for (Mono *p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
    {
        ExtendPoly(&out, MonoClone(p_ptr));
        mem = PolyNeg(&out.first->p);
        out.first->p = mem;
    }
    return out;
}

Poly PolySub(const Poly *p, const Poly *q)
{
    Poly neg = PolyNeg(q);
    Poly out = PolyAdd(p, &neg);
    PolyDestroy(&neg);
    return out;
}

static poly_exp_t MaxExp(poly_exp_t a, poly_exp_t b)
{
    if (a < b)
    {
        return b;
    }
    return a;
}

poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx)
{
    if (PolyIsZero(p))
    {
        return -1;
    }
    if (var_idx == 0)
    {
        return p->first->exp;
    }
    poly_exp_t out = 0;
    for (Mono *ptr = p->last; ptr != NULL; ptr = ptr->prev)
    {
        out = MaxExp(out, PolyDegBy(&ptr->p, var_idx - 1));
    }
    return out;
}

poly_exp_t PolyDeg(const Poly *p)
{

}

bool MonoIsEq(const Mono *a, const Mono *b)
{
    if (a->exp != b->exp)
    {
        return false;
    }
    return PolyIsEq(&a->p, &b->p);
}

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

Poly PolyAt(const Poly *p, poly_coeff_t x)
{
}
