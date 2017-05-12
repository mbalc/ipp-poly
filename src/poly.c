#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "poly.h"


static void LinkMonos(Mono *left, Mono *right)
{
    if (left != NULL)
        left->next = right;
    if (right != NULL)
        right->prev = left;
}

void RemoveMonoFromPoly(Poly *p, Mono *m)
{
    if (m->prev == NULL)
        p->first = m->next;
    if (m->next == NULL)
        p->last = m->prev;
    LinkMonos(m->prev, m->next);
    MonoDestroy(m);
}

void PolyDestroy(Poly *p)
{
    Mono *ptr;
    while (!PolyIsCoeff(p))
    {
        ptr = p->first;
        RemoveMonoFromPoly(p, p->first);
        free(ptr);
    }
}

void PushMonoIntoPoly(Poly *p, Mono *m)
{
    LinkMonos(m, p->first);
    p->first = m;
    if (p->last == NULL)
        p->last = m;
}

Poly PolyClone(const Poly *p)
{
    Poly out = PolyFromCoeff(p->abs_term);
    Mono *buf = MonoMalloc();
    for (Mono *ptr = p->last; ptr != NULL; ptr = ptr->prev)
    {
        *buf = MonoClone(ptr);
        PushMonoIntoPoly(&out, buf);
        buf = MonoMalloc();
    }
    free(buf);
    return out;
}

Poly PolyAdd(const Poly *p, const Poly *q)
{
    Poly out = PolyFromCoeff(p->abs_term + q->abs_term);
    Poly buf = PolyZero();
    Mono *p_ptr = p->last, *q_ptr = q->last;

    while (p_ptr != NULL && q_ptr != NULL)
    {
        if (p_ptr->exp == q_ptr->exp)
        {
            buf = PolyAdd(&p_ptr->p, &q_ptr->p);
            if (!PolyIsZero(&buf))
            {
                PushMonoIntoPoly(&out, MonoMalloc());
                *(out.first) = MonoFromPoly(PolyMalloc(), p_ptr->exp);
                out.first->p = PolyClone(&buf);
            }
            p_ptr = p_ptr->prev;
            q_ptr = q_ptr->prev;
        }
        else if (p_ptr->exp < q_ptr->exp)
        {
            PushMonoIntoPoly(&out, MonoMalloc());
            *(out.first) = MonoClone(p_ptr);
            p_ptr = p_ptr->prev;
        }
        else if (p_ptr->exp > q_ptr->exp)
        {
            PushMonoIntoPoly(&out, MonoMalloc());
            *(out.first) = MonoClone(q_ptr);
            q_ptr = q_ptr->prev;
        }
    }

    if (q_ptr != NULL)
        p_ptr = q_ptr;
    while (p_ptr != NULL)
    {
        PushMonoIntoPoly(&out, MonoMalloc());
        *(out.first) = MonoClone(p_ptr);
        p_ptr = p_ptr->prev;
    }

    PolyDestroy(&buf);

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
    for (unsigned i = 0; i < count; ++i)
        MonoDestroy(&arr[i]);
    qsort(arr, count, sizeof(Mono), CompareMonos);
    Poly out = PolyZero();
    Mono *buf = MonoMalloc();
    *buf = MonoClone(&arr[count - 1]);
    for (int i = count - 2; i > 0; --i)
    {
        if (arr[i].exp == buf->exp)
        {
            buf->p = PolyAdd(&buf->p, &arr[i].p);

        }
        if (arr[i].exp > buf->exp)
        {
            if (!PolyIsZero(&buf->p))
            {
                PushMonoIntoPoly(&out, buf);
                buf = MonoMalloc();
            }
            *buf = MonoClone(&arr[i]);
        }
    }
    PushMonoIntoPoly(&out, buf);
    free(arr);
    return out;
}

Poly PolyCoeffMul(const Poly *p, poly_coeff_t x)
{
    Poly out = PolyFromCoeff(p->abs_term * x);
    for (Mono *p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
    {
        PushMonoIntoPoly(&out, MonoMalloc());
        out.first->exp = p_ptr->exp;
        out.first->p = PolyCoeffMul(&p_ptr->p, x);
    }
    return out;
}

Poly PolyMul(const Poly *p, const Poly *q)
{
    Poly out = PolyZero();
    Poly buf = PolyCoeffMul(q, p->abs_term);
    out = PolyAdd(&out, &buf);
    buf = PolyCoeffMul(p, q->abs_term);
    out = PolyAdd(&out, &buf);
    out.abs_term /= 2;
    for (Mono *p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
    {
        buf = PolyZero();
        for (Mono *q_ptr = q->last; q_ptr != NULL; q_ptr = q_ptr->prev)
        {
            PushMonoIntoPoly(&buf, MonoMalloc());
            buf.first->exp = p_ptr->exp + q_ptr->exp;
            buf.first->p = PolyMul(&p_ptr->p, &q_ptr->p);
        }
        out = PolyAdd(&out, &buf);
    }
    return out;
}

Poly PolyNeg(const Poly *p)
{
    Poly out = PolyFromCoeff(-p->abs_term);
    Poly *ptr;
    for (Mono *p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
    {
        PushMonoIntoPoly(&out, MonoMalloc());
        *(out.first) = MonoClone(p_ptr);
        ptr = &out.first->p;
        out.first->p = PolyNeg(ptr);
        PolyDestroy(ptr);
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

poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx)
{

}

poly_exp_t PolyDeg(const Poly *p)
{

}

bool MonoIsEq(const Mono *a, const Mono *b)
{
    if (a->exp != b->exp)
        return false;
    return PolyIsEq(&a->p, &b->p);
}

bool PolyIsEq(const Poly *p, const Poly *q)
{
    Mono *p_ptr = p->last, *q_ptr = q->last;
    while (p_ptr != NULL && q_ptr != NULL)
    {
        if (!MonoIsEq(p_ptr, q_ptr))
            return false;
        p_ptr = p_ptr->prev;
        q_ptr = q_ptr->prev;
    }
    if (p_ptr != q_ptr)
        return false;
    return p->abs_term == q->abs_term;
}

Poly PolyAt(const Poly *p, poly_coeff_t x)
{

}
