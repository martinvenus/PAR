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
    if (s->top > 0) {
        (s->top)--;
        return (s->array[s->top]);
    } else {
        return -1;
    }
    /*  Equivalent to: return (S->v[--(S->top)]);  */
}

void init(Stack *s, int size) {
    s->top = 0;

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
    return (s->top == 0);
}

void stackPrint(Stack *s) {
    int i;
    if (s->top == 0)
        printf("\nStack is empty.\n");
    else {
        printf("\nStack contents: \n");
        for (i = 0; i < s->top; i++) {
            printf("ID: %i, ", s->array[i]);
        }
        printf("\n");
    }
}

void memoryFreeStack(Stack *s) {
    free(s->array);
}

void DFS_analyse(Stack *s, int** m, int pocetVrcholu) {

    //printf("\nTest2: %i\n", m[0][2]);

    int x = 0;
    int sousede = 0;

    //printf("Vrcholy: %i\n", vrcholy);

    int aktualniVrchol;
    aktualniVrchol = 0;
    m[aktualniVrchol][aktualniVrchol] = 2;
    push(s, aktualniVrchol); // vložím aktuílní vrchol do zásobníku

    // BEGIN: TESTUJI JEDEN SLOUPEC

    while (!(isEmpty(s))) {

        aktualniVrchol = pop(s);
        //printf ("Aktualni vrchol je %i\n", aktualniVrchol.id);
        coloring(aktualniVrchol,pocetVrcholu);

        sousede = 0;
        for (x = 0; x < pocetVrcholu; x++) {
            if (x == aktualniVrchol) continue; // preskocim diagonalu
            if (m[x][x] == 2) continue; // soused jiz byl objeven drive
            if (m[x][aktualniVrchol] == 1) { // jestliže aktuální vrchol sousedí s vrcholem m[x][aktualniVrchol]
                int v;
                v = x;
                m[x][x] = 2;
                push(s, v); // vložím aktuílní vrchol do zásobníku
                //printf("Objeven novy soused %i\n", x);
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