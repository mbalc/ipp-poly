/** @file
   Interfejs stosu wskaźników

   @author Michał Balcerzak <mb385130@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-27
 */

#ifndef __STACK_H__
#define __STACK_H__

#include <stdlib.h>
#include <stdbool.h>


/**
 * Struktura przechowująca stos wskaźników.
 * Prosta struktura stosu oparta na listach pozwalająca przetrzymywać wskaźniki
 * dowolnego typu. W gestii użytkownika pozostaje pamiętanie o typie elementów
 * na nich przetrzymywanych.
 */
typedef struct PointerStack
{
    void *elem_pointer; ///< wskaźnik na element na wierzchu stosu
    struct PointerStack *next_elem; ///< wskaźnik na pamięć przetrzymującą następny element w stosie
    unsigned size; ///< wielkość stosu
} PointerStack;


/**
 * Tworzy nowy, pusty stos wskaźników
 * @return pusty stos wskaźników
 */
PointerStack NewPointerStack();

/**
 * Umieszcza na stosie nowy wskaźnik.
 * @param[in]     new_element : wskaźnik do umieszczenia na stosie
 * @param[in,out] stack       : stos, na którym mamy wykonać operację
 */
void PushOntoStack(void *new_element, PointerStack *stack);

/**
 * Sprawdza czy są jeszcze elementy na stosie.
 * @param[in]  stack : stos, którego stan sprawdzamy
 * @return       czy stos jest niepusty
 */
bool HasStackTop(PointerStack *stack);

/**
 * Zwraca wskaźnik leżący na wierzchu stosu.
 * @param[in]  stack : stos, z którego chcemy pobrać element
 * @return       wskaźnik z wierzchu stosu
 */
void* GetStackTop(PointerStack *stack);

/**
 * Zwraca wskaźnik leżący na wierzchu stosu i usuwa go z niego.
 * @param[in,out]  stack : stos, z którego chcemy pobrać element
 * @return       wskaźnik z wierzchu stosu
 */
void* PollStackTop(PointerStack *stack);

/**
 * Usuwa element leżący na wierzchu stosu.
 * @param[in,out]  stack : stos, z którego chcemy usunąć element
 */
void PopStack(PointerStack *stack);

#endif /* __STACK_H__ */
