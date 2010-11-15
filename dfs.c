/*
 * File:   dfs.c
 * Author: Martin Venuš, Jaroslav Líbal
 *
 * Created on 11. říjen 2010, 14:53
 */

#include <stdio.h>
#include <stdlib.h>
#include "dfs.h"
#include "mbg.h"

extern int my_rank;

void push(Stack *s, int value) {
    if (full(s)) {
        int novaVelikost = 2 * s->size;
        s->array = realloc(s->array, novaVelikost * sizeof (int));

        if (s->array == NULL) {
            printf("Memory reallocation failed.\n");
        } else {
            s->size = novaVelikost;
        }
    }

    s->array[ s->top ] = value;
    (s->top)++;
}

int pop(Stack *s) {
    if (s->top > s->bottom) {
        (s->top)--;
        return (s->array[s->top]);
    } else {
        return -1;
    }
    /*  Equivalent to: return (S->v[--(S->top)]);  */
}

int popBottom(Stack *s) {
    if (s->bottom < s->top) {
        (s->bottom)++;
        return (s->array[s->bottom]);
    } else {
        return -1;
    }
}

int countNodes(Stack *s) {
    return s->top - s->bottom;
}

void init(Stack *s, int size) {
    s->top = 0;
    s->bottom = 0;

    s->array = malloc(size * sizeof (int));

    if (s->array == NULL) {
        s->size = 0;
        printf("Memory allocation failed.\n");
    } else {
        s->size = size;
    }
}

int full(Stack *s) {
    return (s->top >= s->size);
}

int isEmpty(Stack *s) {
    return (s->top == s->bottom);
}

void stackPrint(Stack *s) {
    int i;
    if (s->top == s->bottom)
        printf("\nStack is empty.\n");
    else {
        printf("\nStack contents: \n");
        for (i = s->bottom; i < s->top; i++) {
            printf("ID: %i, ", s->array[i]);
        }
        printf("\n");
    }
}

void memoryFreeStack(Stack *s) {
    free(s->array);
}

void DFS_analyse(Stack *s, int** m, int pocetVrcholu) {

    int x = 0;
    int sousede = 0;
    int aktualniVrchol;

    // Procesor 0 zacne pracovat, ostatni cekaji
    if (my_rank == 0) {
        aktualniVrchol = 0;
        m[aktualniVrchol][aktualniVrchol] = 2;
        push(s, aktualniVrchol); // vložím aktuílní vrchol do zásobníku
        //printf("Vrchol %i vlozen do zasobniku\n", aktualniVrchol);
    }
    else{
        while(isEmpty(s)){
            // TODO: zadat o praci
            // postupne posli zadost o praci vsem procesorum
            // musi cekat na odpoved a teprve kdyz nedostal praci tak se bude ptat dal
        }
    }

    // BEGIN: TESTUJI JEDEN SLOUPEC

    while (1) {

        //printf("Pocet prvku v zasobniku: %i\n", countNodes(s));

        aktualniVrchol = pop(s);
        //printf("Vrchol %i vybran ze zasobniku\n", aktualniVrchol);

        coloring(aktualniVrchol, pocetVrcholu);

        sousede = 0;
        for (x = 0; x < pocetVrcholu; x++) {
            if (x == aktualniVrchol) continue; // preskocim diagonalu
            if (m[x][x] == 2) continue; // soused jiz byl objeven drive
            if (m[x][aktualniVrchol] == 1) { // jestliže aktuální vrchol sousedí s vrcholem m[x][aktualniVrchol]
                int v;
                v = x;
                m[x][x] = 2; // nastavime vrcholu hodnotu 2 (=navstiven)
                push(s, v); // vložím aktuální vrchol do zásobníku
                //printf("Vrchol %i vlozen do zasobniku\n", x);
                sousede++;
            }
        }

        // Pokud mam prazdny zasobnik - vytvoril jsem kompletni konfiguraci
        if (isEmpty(s)){

            break;
           // TODO: Ulozit nejlepsi reseni
           // TODO: Otestovat zda nebyl ukoncen algoritmus (globalni promenna?)
           //       Pokud Ano, pak breaknout, pokud ne pak dalsi ToDo
           // TODO: Poslat pozadavek na praci nahodnemu procesoru + zvysovat citac
        }

    }

    // TODO: Odeslat nejlepsi nalezene reseni

    
}