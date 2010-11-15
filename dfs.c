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
            askForJob(pocetVrcholu, s);

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

            while (1) {
                answerJobRequests(s);
            }
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
void askForJob(int pocetVrcholu, Stack* s) {

    int lenght;
    int nothing = 1;

    int i = 0;
    for (i = 0; i < processSum; i++) {
        if (i != my_rank) {

            MPI_Send(&nothing, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD);
            //printf("Pozadal jsem o praci (Kdo: %d -> Koho: %d)\n", my_rank, i);

            int flag = 0;
            while (flag == 0) {

                MPI_Iprobe(i, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD, &flag, &status);

                if (flag) {
                    MPI_Recv(&lenght, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD, &status);
                    //printf("Dostal jsem odpoved o praci (Jakou: %d, Kdo: %d -> Od koho: %d)\n", lenght, my_rank, i);
                }

                /*
                 * Procesor musí odpovědět na požadavky na práci ostatních procesorů
                 * aby nedošlo k deadlocku
                 */
                answerJobRequests(s);
            }

            if (lenght > 0) {
                break;
            }
        }
    }

    if (lenght > 0) {
        //Inicializace paměti pro konfigurace
        Config* poleKonfiguraci;

        poleKonfiguraci = malloc(lenght * sizeof (Config));
        Prvek* polePrvku;
        polePrvku = malloc(pocetVrcholu * sizeof (Prvek));

        printf("Pocet konfiguraci: %d\n", lenght);

        int j = 0;
        for (j = 0; j < lenght; j++) {
            int pocetBarev = 0;
            int pocetPrvku = 0;
            //Nejprve přijmeme počet barev
            MPI_Recv(&pocetBarev, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_COLORS, MPI_COMM_WORLD, &status);
            printf("Počet barev: %i\n", pocetBarev);
            MPI_Recv(&pocetPrvku, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_ITEMS, MPI_COMM_WORLD, &status);
            printf("Počet prvku: %i\n", pocetPrvku);

            /*
                        int confirationItems = 0;

                        //Poté přijmeme délku pole konfigurací
                        MPI_Recv(&configurationItems, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_ITEMS, MPI_COMM_WORLD, &status);

                        //Poté přijmeme pole konfigurací
                        MPI_Recv(poleKonfiguraci, confirationItems, MPI_INT, i, MESSAGE_JOB_REQUIRE_CONFIGURATION_ARRAY, MPI_COMM_WORLD, &status);
             */
        }

    }
}

/*
 * Odpovíme na případný požadavek na práci
 */
void answerJobRequests(Stack* s) {

    int source;
    int flag;
    int message = -1;

    for (source = 0; source < processSum; source++) {

        if (source != my_rank) {
            /* checking if message has arrived */
            MPI_Iprobe(source, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD, &flag, &status);
            //printf("Testovani pozadavku na praci. Flag: %d, procesor: %d\n", flag, source);
            if (flag) {

                int pocetKonfiguraci = getPocetKonfiguraci();

                //printf("Pred prijetim pozadavek na praci\n");
                /* receiving message by blocking receive */
                MPI_Recv(&message, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD, &status);
                //printf("Prijal jsem pozadavek na praci (Kdo: %d -> Od koho: %d)\n", my_rank, source);

                //printf("Je zasobnik prazdny: %d\n", isEmpty(s));
                // pokud mame predpocitano vice nez jedno reseni a jeste mame co pocitat
                if ((pocetKonfiguraci > 1) && (!isEmpty(s))) {
                    int konfiguraceKOdeslani = (pocetKonfiguraci / 2);
                    int novyPocetKonfiguraci = pocetKonfiguraci - (pocetKonfiguraci / 2);

                    printf("Odesilam pocet konfiguraci: %d\n", konfiguraceKOdeslani);

                    MPI_Send(&konfiguraceKOdeslani, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);

                    int l = 1;
                    for (l = 1; l <= konfiguraceKOdeslani; l++) {
                        int odesilanaKonfigurace;
                        int pocetBarev;
                        int sizeOfPocetPrvku;
                        Prvek* array;

                        odesilanaKonfigurace = novyPocetKonfiguraci + l;
                        pocetBarev = getPocetBarev(odesilanaKonfigurace);

                        array = getKonfigurace(odesilanaKonfigurace);
                        sizeOfPocetPrvku = getPocetPrvku();
                        MPI_Send(&pocetBarev, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_COLORS, MPI_COMM_WORLD);
                        MPI_Send(&sizeOfPocetPrvku, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ITEMS, MPI_COMM_WORLD);
                        /*
                                                MPI_Send(array, sizeOfPocetPrvku * sizeof (Prvek), MPI_BYTE, source, MESSAGE_JOB_REQUIRE_CONFIGURATION_ARRAY, MPI_COMM_WORLD);
                         */

                    }

                    /*
                                        int zasobnikTop;
                                        int zasobnikSize;
                                        int* zasobnikArray;

                                        zasobnikTop = s->top;
                                        zasobnikSize = s->size;
                                        zasobnikArray = s->array;

                                        MPI_Send(&zasobnikSize, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_STACK_SIZE, MPI_COMM_WORLD);
                                        MPI_Send(&zasobnikTop, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_STACK_TOP, MPI_COMM_WORLD);
                                        MPI_Send(zasobnikArray, zasobnikSize, MPI_INT, source, MESSAGE_JOB_REQUIRE_STACK_ARRAY, MPI_COMM_WORLD);
                     */

                    setPocetKonfiguraci(novyPocetKonfiguraci);

                } else {
                    // odpovez ze nemam praci

                    //TODO: Upravit nothing
                    int nothing = NO_JOB;
                    MPI_Send(&nothing, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);
                    int konfiguraceKOdeslani = 0;
                    MPI_Send(&konfiguraceKOdeslani, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);

                    //printf("Odpovedel jsem ze namam praci k predani (Kdo: %d -> Komu: %d)\n", my_rank, source);

                }

            }
        }
    }
}


