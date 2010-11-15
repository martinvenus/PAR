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
    if (s->top > 0) {
        (s->top)--;
        return (s->array[s->top]);
    } else {
        return -1;
    }
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

        answerJobRequests(s);

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
    int length = NO_JOB;

    int i = 0;
    for (i = 0; i < processSum; i++) {

        MPI_Send(1, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD);
        MPI_Recv(&length, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD, &status);

        if (length > 0) {
            break;
        }
    }

    if (length > 0) {
        int pocetBarev = -1;

        /*
         * Inicializace paměti pro konfigurace
         */
        poleKonfiguraci = malloc(length * sizeof (Config));
        Prvek* polePrvku;
        polePrvku = malloc(pocetVrcholu * sizeof (Prvek));

        int j = 0;
        for (j = 0; j < length; j++) {
            //Nejprve přijmeme počet barev
            MPI_Recv(&pocetBarev, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_COLORS, MPI_COMM_WORLD, &status);

            int confirationItems = 0;

            //Poté přijmeme délku pole konfigurací
            MPI_Recv(&configurationItems, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_ITEMS, MPI_COMM_WORLD, &status);

            //Poté přijmeme pole konfigurací
            MPI_Recv(poleKonfiguraci, confirationItems, MPI_INT, i, MESSAGE_JOB_REQUIRE_CONFIGURATION_ARRAY, MPI_COMM_WORLD, &status);
        }
    }
}

/*
 * Odpovíme na případný požadavek na práci
 */
void answerJobRequests(Stack* s) {

    int source;
    int flag;
    int message;

    for (source = 0; source < processSum;) {

        if (source != my_rank) {
            /* checking if message has arrived */
            MPI_Iprobe(MPI_ANY_SOURCE, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD, &flag, &status);
            if (flag) {

                int pocetKonfiguraci = getPocetKonfiguraci();

                /* receiving message by blocking receive */
                MPI_Recv(&message, 1, MPI_CHAR, MPI_ANY_SOURCE, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD, &status);

                // pokud mame predpocitano vice nez jedno reseni a jeste mame co pocitat
                if ((pocetKonfiguraci > 1) && (!isEmpty(s))) {

                    int konfiguraceKOdeslani = (pocetKonfiguraci / 2);
                    int novyPocetKonfiguraci = pocetKonfiguraci - (pocetKonfiguraci / 2);

                    MPI_Send(&konfiguraceKOdeslani, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);

                    int l = 1;
                    for (l = 1; l <= konfiguraceKOdeslani; l++) {
                        int odesilanaKonfigurace;
                        int pocetBarev;
                        int sizeOfPocetPrvku;
                        Prvek* array;

                        int zasobnikTop;
                        int zasobnikSize;
                        int* zasobnikArray;

                        zasobnikTop = s->top;
                        zasobnikSize = s->size;
                        zasobnikArray = s->array;

                        odesilanaKonfigurace = novyPocetKonfiguraci + l;
                        pocetBarev = getPocetBarev(odesilanaKonfigurace);
                        array = getKonfigurace(odesilanaKonfigurace);
                        sizeOfPocetPrvku = getPocetPrvku() * sizeof (Prvek);
                        MPI_Send(&pocetBarev, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_COLORS, MPI_COMM_WORLD);
                        MPI_Send(&sizeOfPocetPrvku, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ITEMS, MPI_COMM_WORLD);
                        MPI_Send(array, sizeOfPocetPrvku, MPI_BYTE, source, MESSAGE_JOB_REQUIRE_CONFIGURATION_ARRAY, MPI_COMM_WORLD);

                        MPI_Send(&zasobnikSize, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_STACK_SIZE, MPI_COMM_WORLD);
                        MPI_Send(&zasobnikTop, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_STACK_TOP, MPI_COMM_WORLD);
                        MPI_Send(zasobnikArray, zasobnikSize, MPI_INT, source, MESSAGE_JOB_REQUIRE_STACK_ARRAY, MPI_COMM_WORLD);

                    }

                    setPocetKonfiguraci(novyPocetKonfiguraci);

                } else {
                    // odpovez ze nemam praci
                    MPI_Send(NO_JOB, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);
                }

                source++;

            }
        }
    }
}


