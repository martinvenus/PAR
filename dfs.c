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

    //printf("Vrcholy: %i\n", vrcholy);

    int aktualniVrchol;
    aktualniVrchol = 0;
    m[aktualniVrchol][aktualniVrchol] = 2;
    push(s, aktualniVrchol); // vložím aktuílní vrchol do zásobníku
    //printf("Vrchol %i vlozen do zasobniku\n", aktualniVrchol);

    // BEGIN: TESTUJI JEDEN SLOUPEC

    while (!(isEmpty(s))) {

        //printf("Pocet prvku v zasobniku: %i\n", countNodes(s));

        aktualniVrchol = pop(s);
        //printf("Vrchol %i vybran ze zasobniku\n", aktualniVrchol);


/*
        // BEGIN IF SOMEONE NEEDS JOB

        // pokud jsme pred chvili nevybrali posledni prvek, posleme prvky
        // jinak posleme zpravu ze mame prazdny zasobnik
        if (!(isEmpty(s))) {

            int i = 0;
            printf("--------------\n");
            for (i = s->bottom; i <= (countNodes(s) / 2); i++) {
                printf("ODEBERU: %i\n", popBottom(s));
            }
            printf("--------------\n");

        }
        else{
            printf("Mam prazdny zasobnik.\n");
        }

        // END IF SOMEONE NEEDS JOB
*/

        coloring(aktualniVrchol, pocetVrcholu);

        sousede = 0;
        for (x = 0; x < pocetVrcholu; x++) {
            if (x == aktualniVrchol) continue; // preskocim diagonalu
            if (m[x][x] == 2) continue; // soused jiz byl objeven drive
            if (m[x][aktualniVrchol] == 1) { // jestliže aktuální vrchol sousedí s vrcholem m[x][aktualniVrchol]
                int v;
                v = x;
                m[x][x] = 2;
                push(s, v); // vložím aktuílní vrchol do zásobníku
                //printf("Vrchol %i vlozen do zasobniku\n", x);
                sousede++;
            }
        }

        //printf("\nVrchol %i ma %i novych sousedu\n", aktualniVrchol.id, sousede);

        if (sousede == 0) {
            //printf("Nalezen vrchol %i.\n", aktualniVrchol.id);
        }

        // END: TESTUJI JEDEN SLOUPEC

    }

    //stackPrint(s);

    //printf("\nDFS done\n");

}