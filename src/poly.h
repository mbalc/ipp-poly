/** @file
   Interfejs klasy wielomianów

   @author Jakub Pawlewicz <pan@mimuw.edu.pl>,\n
   Michał Balcerzak <mb385130@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-04-24, 2017-05-13
 */


#ifndef __POLY_H__
#define __POLY_H__

#include <stdbool.h>
#include <stdlib.h>


/** Typ współczynników wielomianu */
typedef long poly_coeff_t;

/** Typ wykładników wielomianu */
typedef long poly_exp_t;

typedef struct Mono Mono;

/**
 * Struktura przechowująca wielomian.
 * Wielomian ma postać @f$(m_1 + m_2 + … + m_n) + b@f$ gdzie @f$m_1…m_n@f$
 * są jednomianami. Na wielomian składa się uporządkowana lista jednomianów oraz
 * wyraz wolny. Porządek tej listy jest ściśle malejący względem wykładników
 * jednomianów. Pierwszy jednomian listy ma największy wykładnik, ostatni -
 * najmniejszy. Struktura Poly przechowuje jedynie wskaźniki na skrajne elementy
 * tej listy. Wielomian może być wielomianem stałym - wtedy lista jednomianów
 * jest listą pustą. W takim przypadku oba wskaźniki na skrajne elementy listy
 * wskazują na NULL. Wszelkie składniki stałe (niezależne od zmiennych)
 * są pamiętane w wyrazie wolnym.
 */
typedef struct Poly
{
    Mono *first; ///< pierwszy element listy jednomianów (największy wykładnik)
    Mono *last; ///< ostatni element listy jednomianów (najmniejszy wykładnik)
    poly_coeff_t abs_term; ///< wartość wyrazu wolnego
} Poly;


/**
 * Struktura przechowująca jednomian.
 * Jednomian ma postać @f$px^e@f$.
 * Współczynnik `p` może też być wielomianem.
 * Będzie on traktowany jako wielomian nad kolejną zmienną (nie nad x).
 * Jednomian jest elementem uporzadkowanej listy elementów pewnego wielomianu.
 * Zawiera on wskaźniki na sąsiednie jej elementy.
 * Skrajne elementy listy wskazują na NULL jako sąsiada
 */
typedef struct Mono
{
    Poly p; ///< współczynnik
    poly_exp_t exp; ///< wykładnik
    Mono *prev; ///< poprzedni element listy (większy wykładnik)
    Mono *next; ///< następny element listy (mniejszy wykładnik)
} Mono;


/**@name Konstruktory
   @{*/

/**
 * Tworzy wielomian stały, który jest współczynnikiem.
 * @param[in] c : wartość współczynnika
 * @return wielomian stały o wartości @p c
 */
static inline Poly PolyFromCoeff(poly_coeff_t c)
{
    return (Poly) {.first = NULL, .last = NULL, .abs_term = c};
}

/**
 * Tworzy wielomian tożsamościowo równy zeru.
 * @return wielomian stały o wartości '0'
 */
static inline Poly PolyZero()
{
    return PolyFromCoeff(0);
}

/**
 * Tworzy jednomian `p * x^e`.
 * Przejmuje na własność zawartość struktury wskazywanej przez @p p.
 * @param[in] p : wielomian - współczynnik jednomianu
 * @param[in] e : wykładnik
 * @return jednomian `p * x^e`
 */
static inline Mono MonoFromPoly(const Poly *p, poly_exp_t e)
{
    return (Mono) {.p = *p, .exp = e, .prev = NULL, .next = NULL};
}


/**
 * Sumuje listę jednomianów i tworzy z nich wielomian.
 * Przejmuje na własność zawartość tablicy @p monos.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
Poly PolyAddMonos(unsigned count, const Mono monos[]);

/*}@**/


/**@name Konstruktory kopiujące
   @{*/

/**
 * Robi pełną, głęboką kopię wielomianu.
 * @param[in] p : wielomian
 * @return skopiowany wielomian
 */
Poly PolyClone(const Poly *p);

/**
 * Robi pełną, głęboką kopię jednomianu.
 * @param[in] m : jednomian
 * @return skopiowany jednomian
 */
static inline Mono MonoClone(const Mono *m)
{
    return (Mono) {.p = PolyClone(&(m->p)), .exp = m->exp};
}

/*}@**/


/**@name Destruktory
   @{*/

/**
 * Usuwa wielomian z pamięci.
 * @param[in] p : wielomian
 */
void PolyDestroy(Poly *p);

/**
 * Usuwa jednomian z pamięci.
 * @param[in] m : jednomian
 */
static inline void MonoDestroy(Mono *m)
{
    PolyDestroy(&m->p);
}

/*}@**/


/**@name Operatory
   @{*/

/**
 * Dodaje dwa wielomiany.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p + q`
 */
Poly PolyAdd(const Poly *p, const Poly *q);

/**
 * Przemnaża dany wielomian przez stałą @p x
 * @param[in]  p : wielomian
 * @param[in]  x : stała
 * @return 'p * x'
 */
Poly PolyCoeffMul(const Poly *p, poly_coeff_t x);

/**
 * Mnoży dwa wielomiany.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p * q`
 */
Poly PolyMul(const Poly *p, const Poly *q);

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian
 * @return `-p`
 */
Poly PolyNeg(const Poly *p);

/**
 * Odejmuje wielomian od wielomianu.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p - q`
 */
Poly PolySub(const Poly *p, const Poly *q);

/*}@**/


/**@name Komparatory
   @{*/

/**
 * Sprawdza, czy wielomian jest współczynnikiem.
 * @param[in] p : wielomian
 * @return Czy wielomian jest współczynnikiem?
 */
static inline bool PolyIsCoeff(const Poly *p)
{
    return (p->first == NULL);
}

/**
 * Sprawdza, czy wielomian jest tożsamościowo równy zeru.
 * @param[in] p : wielomian
 * @return Czy wielomian jest równy zero?
 */
static inline bool PolyIsZero(const Poly *p)
{
    return (PolyIsCoeff(p) && p->abs_term == 0);
}

/**
 * Sprawdza równość dwóch wielomianów.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p = q`
 */
bool PolyIsEq(const Poly *p, const Poly *q);

/*}@**/


/**@name Funkcje obliczeniowe
   @{*/

/**
 * Zwraca stopień wielomianu ze względu na zadaną zmienną (-1 dla wielomianu
 * tożsamościowo równego zeru).
 * Zmienne indeksowane są od 0.
 * Zmienna o indeksie 0 oznacza zmienną główną tego wielomianu.
 * Większe indeksy oznaczają zmienne wielomianów znajdujących się
 * we współczynnikach.
 * @param[in] p : wielomian
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx
 */
poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx);

/**
 * Zwraca stopień wielomianu (-1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDeg(const Poly *p);

/**
 * Wylicza wartość wielomianu w punkcie @p x.
 * Wstawia pod pierwszą zmienną wielomianu wartość @p x.
 * W wyniku może powstać wielomian, jeśli współczynniki są wielomianem
 * i zmniejszane są indeksy zmiennych w takim wielomianie o jeden.
 * Formalnie dla wielomianu @f$p(x_0, x_1, x_2, \ldots)@f$ wynikiem jest
 * wielomian @f$p(x, x_0, x_1, \ldots)@f$.
 * @param[in] p : wielomian
 * @param[in] x : wartość pierwszej ze zmiennych
 * @return @f$p(x, x_0, x_1, \ldots)@f$
 */
Poly PolyAt(const Poly *p, poly_coeff_t x);

/*}@**/


/**@name Funkcje pomocnicze
   @{*/

/**
 * Wypisuje na standardowe wyjście zawartość struktury wielomianu.
 * Kolejne zmienne wielomianu sa nazywane kolejnymi literami alfabetu
 * łacińskiego.
 * @param[in] p : wielomian
 */
void PrintPoly(const Poly *p);

/*}@**/

#endif /* __POLY_H__ */
