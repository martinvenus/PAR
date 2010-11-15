/*
 * File:   dfs.c
 * Author: Martin Venuš, Jaroslav Líbal
 *
 * Created on 11. říjen 2010, 14:53
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "dfs.h"
#include "mbg.h"

#define HAVE_JOB 1
#define NO_JOB 0

extern int my_rank;
extern int processSum;
extern MPI_Status status;

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
    } else {

        while (isEmpty(s)) {
            // Požádáme ostatní procesory o práci
            askForJob();

            // postupne posli zadost o praci vsem procesorum
            // musi cekat na odpoved a teprve kdyz nedostal praci tak se bude ptat dal

        }

    }

    // BEGIN: TESTUJI JEDEN SLOUPEC

    while (1) {

        answerJobRequests();

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
        if (isEmpty(s)) {

            break;
            // TODO: Ulozit nejlepsi reseni
            // TODO: Otestovat zda nebyl ukoncen algoritmus (globalni promenna?)
            //       Pokud Ano, pak breaknout, pokud ne pak dalsi ToDo
            // TODO: Poslat pozadavek na praci nahodnemu procesoru + zvysovat citac
        }

    }

    // TODO: Odeslat nejlepsi nalezene reseni
}

/*
 * Požádáme ostatní procesory o práci a čekáme na jejich odpověď
 * Žádání o práci probíhá postupně od procesoru s nejnižším ID k nejvyššímu
 */
void askForJob() {
    int data = NO_JOB;

    int i = 0;
    for (i = 0; i < processSum; i++) {

        MPI_Send(1, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD);
        MPI_Recv(&data, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD, &status);

        if (data == HAVE_JOB) {
            break;
        }
    }

    if (data == HAVE_JOB) {
        //Přijmeme práci
    }
}

/*
 * Odpovíme na případný požadavek na práci
 */
void answerJobRequests() {

    int source;
    int flag;
    int message;

    for (source = 0; source < processSum;) {

        if (source != my_rank) {
            /* checking if message has arrived */
            MPI_Iprobe(MPI_ANY_SOURCE, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD, &flag, &status);
            if (flag) {
                /* receiving message by blocking receive */
                MPI_Recv(&message, LENGTH, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                // pokud mame predpocitano vice nez jedno reseni a jeste mame co pocitat
                if ((getPocetKonfiguraci() > 1) && (!isEmpty(s))) {

                    int konfiguraceKOdeslani = (pocetKonfiguraci / 2);
                    int novyPocetKonfiguraci = pocetKonfiguraci - (pocetKonfiguraci / 2);

                    MPI_Send(&konfiguraceKOdeslani, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);

                    /*
                                        int l = 0;
                                        for (l = 0; l < konfiguraceKOdeslani; l++) {
                                            MPI_Send(maticeSousednosti[l], pocetVrcholu, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);
                                        }
                     */

                }
                else{
                    // odpovez ze nemam praci
                    MPI_Send(0, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);
                }

                source++;

            }
        }
    }
}


