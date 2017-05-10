#include <stdbool.h>
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
    while (!PolyIsCoeff(p))
    {
        RemoveMonoFromPoly(p, p->first);
    }
}

Poly PolyClone(const Poly *p)
{
    Poly out = PolyFromCoeff(p->abs_term);
    if (!PolyIsCoeff(p))
    {
        Mono* iter = p->first;
        Mono* head = MonoMalloc();
        Mono* toAdd = MonoMalloc();
        *head = MonoClone(iter);
        out.first = head;
        LinkMonos(NULL, out.first);
        while (iter != p->last)
        {
            iter = iter->next;
            *toAdd = MonoClone(iter);
            LinkMonos(head, toAdd);
            head = toAdd;
        }
        out.last = head;
        LinkMonos(out.last, NULL);
    }
    return out;
}

void PushMonoIntoPoly(Poly *p, Mono *m)
{
    LinkMonos(m, p->first);
    p->first = m;
    if (p->last == NULL)
        p->last = m;
}

Poly PolyAdd(const Poly *p, const Poly *q)
{
    Poly out = PolyFromCoeff(p->abs_term + q->abs_term);
    Poly buff = PolyZero();
    Mono *p_ptr = p->last, *q_ptr = q->last;

    while (p_ptr != NULL && q_ptr != NULL)
    {
        if (p_ptr->exp == q_ptr->exp)
        {
            buff = PolyAdd(&p_ptr->p, &q_ptr->p);
            if (!PolyIsZero(&buff))
            {
                PushMonoIntoPoly(&out, MonoMalloc());
                *(out.first) = MonoFromPoly(PolyMalloc(), p_ptr->exp);
                out.first->p = PolyClone(&buff);
                p_ptr = p_ptr->prev;
                q_ptr = q_ptr->prev;

            }
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

    PolyDestroy(&buff);

    return out;
}

Poly PolyAddMonos(unsigned count, const Mono monos[])
{

}

Poly PolyCoeffMul(const Poly *p, poly_coeff_t x)
{
    Poly out = PolyFromCoeff(p->abs_term * x);
    for (Mono* p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
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
    Poly buff = PolyCoeffMul(q, p->abs_term);
    out = PolyAdd(&out, &buff);
    buff = PolyCoeffMul(p, q->abs_term);
    out = PolyAdd(&out, &buff);
    out.abs_term /= 2;
    for (Mono* p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
    {
        buff = PolyZero();
        for (Mono* q_ptr = q->last; q_ptr != NULL; q_ptr = q_ptr->prev)
        {
            PushMonoIntoPoly(&buff, MonoMalloc());
            buff.first->exp = p_ptr->exp + q_ptr->exp;
            buff.first->p = PolyMul(&p_ptr->p, &q_ptr->p);
        }
        out = PolyAdd(&out, &buff);
    }
    return out;
}

Poly PolyNeg(const Poly *p)
{
    Poly out = PolyFromCoeff(-p->abs_term);
    Poly* ptr;
    for (Mono* p_ptr = p->last; p_ptr != NULL; p_ptr = p_ptr->prev)
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
