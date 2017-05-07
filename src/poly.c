#include <stdbool.h>
#include <stdlib.h>
#include "poly.h"


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
        Mono* head = malloc (sizeof (Mono));
        Mono* toAdd = malloc (sizeof (Mono));
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


Poly PolyAdd(const Poly *p, const Poly *q);

Poly PolyAddMonos(unsigned count, const Mono monos[]);

Poly PolyMul(const Poly *p, const Poly *q);

Poly PolyNeg(const Poly *p);

Poly PolySub(const Poly *p, const Poly *q);

poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx);

poly_exp_t PolyDeg(const Poly *p);

bool PolyIsEq(const Poly *p, const Poly *q);

Poly PolyAt(const Poly *p, poly_coeff_t x);
