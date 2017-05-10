#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "poly.h"



static inline Poly* PolyMalloc () {
    Poly* out = (Poly*) malloc (sizeof (Poly));
    assert (out);
    return out;
}

static inline Mono* MonoMalloc () {
    Mono* out = (Mono*) malloc (sizeof (Mono));
    assert (out);
    return out;
}


static void LinkMonos(Mono *left, Mono *right)
{
    if (left != NULL) left->next = right;
    if (right != NULL) right->prev = left;
}

void RemoveMonoFromPoly (Poly *p, Mono *m) {
    if (m->prev == NULL) p->first = m->next;
    if (m->next == NULL) p->last = m->prev;
    LinkMonos (m->prev, m->next);
    MonoDestroy (m);
}

void PolyDestroy(Poly *p)
{
    while (!PolyIsCoeff(p)) {
        RemoveMonoFromPoly (p, p->first);
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

void PushMonoIntoPoly (Poly *p, Mono *m) {
    LinkMonos (m, p->first);
    p->first = m;
    if (p->last == NULL) p->last = m;
}

Poly PolyAdd(const Poly *p, const Poly *q)
{
    Poly out = PolyFromCoeff (p->abs_term + q->abs_term);
    Poly* buff = PolyMalloc();
    Mono *PPtr = p->last, *QPtr = q->last;

    while (PPtr != NULL && QPtr != NULL)
    {
        if (PPtr->exp == QPtr->exp)
        {
            *buff = PolyAdd(&PPtr->p, &QPtr->p);
            if (!PolyIsZero(buff))
            {
                PushMonoIntoPoly (&out, MonoMalloc());
                *(out.first) = MonoFromPoly (PolyMalloc(), PPtr->exp);
                out.first->p = PolyClone (buff);
                PPtr = PPtr->prev;
                QPtr = QPtr->prev;

            }
        }
        else if (PPtr->exp < QPtr->exp)
        {
            PushMonoIntoPoly (&out, MonoMalloc());
            *(out.first) = MonoClone (PPtr);
            PPtr = PPtr->prev;
        }
        else if (PPtr->exp > QPtr->exp)
        {
            PushMonoIntoPoly (&out, MonoMalloc());
            *(out.first) = MonoClone (QPtr);
            QPtr = QPtr->prev;
        }
    }

    if (QPtr != NULL) PPtr = QPtr;
    while (PPtr != NULL)
    {
        PushMonoIntoPoly (&out, MonoMalloc());
        *(out.first) = MonoClone (PPtr);
        PPtr = PPtr->prev;
    }

    PolyDestroy (buff);

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
    Poly out = PolyFromCoeff (-p->abs_term);
    Poly* buff;
    for (Mono* PPtr = p->last; PPtr != NULL; PPtr = PPtr->prev) {
        PushMonoIntoPoly (&out, MonoMalloc());
        *(out.first) = MonoClone (PPtr);
        buff = &out.first->p;
        out.first->p = PolyNeg(buff);
        PolyDestroy(buff);
    }
    return out;
}

Poly PolySub(const Poly *p, const Poly *q)
{
    Poly neg = PolyNeg (q);
    Poly out = PolyAdd (p, &neg);
    PolyDestroy (&neg);
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
    if (a->exp != b->exp) return false;
    return PolyIsEq(&a->p, &b->p);
}

bool PolyIsEq(const Poly *p, const Poly *q)
{
    Mono *PPtr = p->last, *QPtr = q->last;
    while (PPtr != NULL && QPtr != NULL) {
        if (!MonoIsEq (PPtr, QPtr)) return false;
        PPtr = PPtr->prev;
        QPtr = QPtr->prev;
    }
    if (PPtr != QPtr) return false;
    return p->abs_term == q->abs_term;
}

Poly PolyAt(const Poly *p, poly_coeff_t x)
{

}
