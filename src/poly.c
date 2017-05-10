#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "poly.h"


Poly* PolyMalloc () {
    Poly* out = malloc (sizeof (Poly));
    assert (out);
    return out;
}

Mono* MonoMalloc () {
    Mono* out = malloc (sizeof (Mono));
    assert (out);
    return out;
}

void PolyDestroy(Poly *p)
{
    if (!PolyIsCoeff(p))
    {
        Mono* toDestroy;
        while (p->first != p->last)
        {
            toDestroy = p->first;
            p->first = toDestroy->next;
            MonoDestroy(toDestroy);
        }
        MonoDestroy(p->last);
    }
    free(p);
}


static void LinkMonos(Mono *left, Mono *right)
{
    if (left != NULL) left->next = right;
    if (right != NULL) right->prev = left;
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

void RemoveMonoFromPoly (Poly *p, Mono *m) {
    if (m->prev == NULL) p->first = m->next;
    if (m->next == NULL) p->last = m->prev;
    LinkMonos (m->prev, m->next);
    MonoDestroy (m);
}

void PushMonoIntoPoly (Poly *p, Mono *m) {
    LinkMonos (m, p->first);
    p->first = m;
    if (p->last == NULL) p->last = m;
}

Poly PolyAdd(const Poly *p, const Poly *q)
{
    Poly out = PolyFromCoeff (p->abs_term + q->abs_term);
    Poly* add = PolyMalloc();
    Mono *pPtr = p->last, *qPtr = q->last;

    while (p != NULL || q != NULL)
    {
        if (pPtr->exp == qPtr->exp)
        {
            *add = PolyAdd(&pPtr->p, &qPtr->p);
            if (!PolyIsZero(add))
            {
                PushMonoIntoPoly (&out, MonoMalloc());
                *(out.first) = MonoFromPoly (PolyMalloc(), pPtr->exp);
                out.first->p = PolyClone (add);
                pPtr = pPtr->prev;
                qPtr = qPtr->prev;

            }
        }
        if (pPtr->exp < qPtr->exp)
        {
            PushMonoIntoPoly (&out, MonoMalloc());
            *(out.first) = MonoClone (pPtr);
            pPtr = pPtr->prev;
        }
        if (pPtr->exp > qPtr->exp)
        {
            PushMonoIntoPoly (&out, MonoMalloc());
            *(out.first) = MonoClone (qPtr);
            qPtr = qPtr->prev;
        }
    }

    if (qPtr != NULL) pPtr = qPtr;
    while (pPtr != NULL) {
        PushMonoIntoPoly (&out, MonoMalloc());
        *(out.first) = MonoClone (pPtr);
        pPtr = pPtr->prev;
    }

    return out;
}

Poly PolyAddMonos(unsigned count, const Mono monos[])
{

}

Poly PolyMul(const Poly *p, const Poly *q)
{

}

Poly PolyNeg(const Poly *p)
{

}

Poly PolySub(const Poly *p, const Poly *q)
{

}

poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx)
{

}

poly_exp_t PolyDeg(const Poly *p)
{

}

bool PolyIsEq(const Poly *p, const Poly *q)
{

}

Poly PolyAt(const Poly *p, poly_coeff_t x)
{

}
