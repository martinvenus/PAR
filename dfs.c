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
extern int root;

extern int processSum;
int* diag;
extern MPI_Status status;
extern MPI_Request request;
int my_color = PESEK_WHITE;
int algoritmusUkoncen = 0;

int pesek_sent = 0;
int pokusyPrijetiPrace = 0;

int bestColorsReceived = 9999;
int bestColorReceivedProcessor = -1;

/*
 Vlozeni dat do zasobniku
 */
void push(Stack *s, int value) {
    if (full(s)) {
        int novaVelikost = 2 * s->size; // dynamicka realokace pameti
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

/*
 Vyber dat ze zasobniku
 */
int pop(Stack *s) {
    if (s->top > 0) {
        (s->top)--;
        return (s->array[s->top]);
    } else {
        return -1;
    }
}

/*
 Inicializace zasobniku
 */
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

/*
 Overeni zda neni zasobnik plny
 */
int full(Stack *s) {
    return (s->top >= s->size);
}

/*
 Overeni zda neni zasobnik prazdny
 */
int isEmpty(Stack *s) {
    return (s->top == 0);
}

/*
 Vypsani zasobniku
 */
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

/*
 Uvolneni pameti zasobniku
 */
void memoryFreeStack(Stack *s) {
    free(s->array);
}

/*
 Hlavni funkce - DFS pruchod grafem
 */
void DFS_analyse(Stack *s, int** m, int pocetVrcholu) {

    int x = 0;
    int sousede = 0;
    int aktualniVrchol;
    int bestColors = -1;

    // alokace a inicializace pole pro ukladani diagonaly
    diag = malloc(pocetVrcholu * sizeof (int));
    int k = 0;
    for (k = 0; k < pocetVrcholu; k++) {
        diag[k] = 0;
    }

    // Procesor 0 zacne pracovat, ostatni cekaji
    if (my_rank == root) {
        aktualniVrchol = 0;
        diag[0] = 2;
        push(s, aktualniVrchol); // vložím aktuílní vrchol do zásobníku
        //printf("Vrchol %i vlozen do zasobniku\n", aktualniVrchol);
    } else {

        // pokud je zasobnik prazdny (procesor nema praci)
        while (isEmpty(s)) {

            // Požádáme ostatní procesory o práci
            askForJob(pocetVrcholu, s);

            // ověříme, zda nepřišel ukončovací pešek
            pesekOstatni();

            if (algoritmusUkoncen == 1) {
                break;
            }

        }

    }
    // BEGIN: TESTUJI JEDEN SLOUPEC

    while (1) {
        if (algoritmusUkoncen == 1) {
            break;
        }


        answerJobRequests(s, pocetVrcholu);

        //printf("Pocet prvku v zasobniku: %i\n", countNodes(s));


        //printf("\nJsem procesor %d popuju z vrcholu coz je: %d\n", my_rank, s->top);
        //printf("\nA na topu mam: %d\n", s->array[s->top - 1]);
        //printf("\nPricemz velikost zasobniku je: %d\n", s->size);

        // vybereme vrchol ktery chceme obarvit ze zasobniku
        aktualniVrchol = pop(s);

        //printf("\nPop se povedl - popnul jsem: %d\n", aktualniVrchol);
        //printf("Vrchol %i vybran ze zasobniku\n", aktualniVrchol);

        //printf("\nPocet vrcholu: %d\n", pocetVrcholu);

        // obarvime vrchol
        coloring(aktualniVrchol, pocetVrcholu);


        // vyhledame nenavstivene sousedy vrcholu a ty vlozime do zasobniku
        sousede = 0;
        for (x = 0; x < pocetVrcholu; x++) {
            if (x == aktualniVrchol) continue; // preskocim diagonalu
            if (diag[x] == 2) continue; // soused jiz byl objeven drive
            if (m[x][aktualniVrchol] == 1) { // jestliže aktuální vrchol sousedí s vrcholem m[x][aktualniVrchol]
                int v;
                v = x;
                diag[x] = 2; // nastavime vrcholu hodnotu 2 (=navstiven)
                push(s, v); // vložím aktuální vrchol do zásobníku
                //printf("Vrchol %i vlozen do zasobniku\n", x);
                sousede++;
            }
        }


        // Pokud mam prazdny zasobnik - vytvoril jsem kompletni konfiguraci
        if (isEmpty(s)) {

            //printf("Procesor %d hlasi: Dosla mi prace\n", my_rank);


            // ulozime si nejlepsi reseni
            findBestColouring();
            if ((bestColors > getBestColors()) || (bestColors == -1)) {
                bestColors = getBestColors();
                setBestSolutionToMatrix(pocetVrcholu);
            }

            // obarvime procesor pro potreby posilani peska
            if (my_color == PESEK_BROWN) {
                my_color = PESEK_BLACK;
            } else {
                my_color = PESEK_WHITE;
            }

            // hledame novou praci
            while (1) {

                // odpovime na pozadavky na praci (nemame praci)
                answerJobRequests(s, pocetVrcholu);

                if (algoritmusUkoncen == 0) {
                    // TODO: Ptat se na praci
                    //askForJob(pocetVrcholu, s);
                }

                // pokud jsme dostali praci, obarvujeme
                if (!isEmpty(s)) {
                    //printf("Procesor %d dostal praci.\n", my_rank);
                    //answerJobRequests(s,pocetVrcholu);
                    break;
                }

                // zkontrolijeme posilani pesku
                pesekRoot();
                pesekOstatni();

                if (algoritmusUkoncen == 1) {
                    break;
                }

            }

        }
    }

    /*
     * Odesíláme nejlepší řešení
     */
    if (my_rank == root) {
        int i = 0;
        for (i = 1; i < processSum; i++) {
            int bestColorsReceivedPom = 9999;
            MPI_Recv(&bestColorsReceivedPom, 1, MPI_INT, i, MESSAGE_FINISH_BEST_COLORS, MPI_COMM_WORLD, &status);

            if (bestColorsReceivedPom < bestColorsReceived) {
                bestColorsReceived = bestColorsReceivedPom;
                bestColorReceivedProcessor = i;
            }

            //printf("Nejmensi pocet barev: %d na procesoru: %d", bestColorsReceived, bestColorReceivedProcessor);

            if (bestColorsReceived <= bestColors) {
                int best = 0;
                MPI_Send(&best, 1, MPI_INT, i, MESSAGE_FINISH_BEST, MPI_COMM_WORLD);
            } else {
                MPI_Send(&bestColorReceivedProcessor, 1, MPI_INT, i, MESSAGE_FINISH_BEST, MPI_COMM_WORLD);
            }
        }

        if (bestColorsReceived <= bestColors) {
            printBestSolution(pocetVrcholu);
        }
    } else {
        MPI_Send(&bestColors, 1, MPI_INT, 0, MESSAGE_FINISH_BEST_COLORS, MPI_COMM_WORLD);

        int best = -1;
        MPI_Recv(&best, 1, MPI_INT, 0, MESSAGE_FINISH_BEST, MPI_COMM_WORLD, &status);

        if (best == my_rank) {
            printBestSolution(pocetVrcholu);
        }
    }

    // uvolnime pole pro uchovani diagonaly
    free(diag);

}

/*
 * Požádáme ostatní procesory o práci a čekáme na jejich odpověď
 * Žádání o práci probíhá postupně od procesoru s nejnižším ID k nejvyššímu
 */
void askForJob(int pocetVrcholu, Stack* s) {

    int lenght;
    int nothing = 1;

    // vsechny procesory (krome sebe sama) zkousime postupne zadat o praci
    int i = 0;
    for (i = 0; i < processSum; i++) {
        if (i != my_rank) {

            MPI_Send(&nothing, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD);
            //printf("Pozadal jsem o praci (Kdo: %d -> Koho: %d)\n", my_rank, i);

            int flag = 0;
            int citac = 0;
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
                answerJobRequests(s, pocetVrcholu);

                // kontrolujeme pesky
                pesekRoot();
                pesekOstatni();

                if (algoritmusUkoncen == 1) {
                    break;
                }

            }

            if (lenght > 0) {
                break;
            }
        }
    }

    if (lenght > 0) {
        //Inicializace paměti pro konfigurace
        Config* poleKonfiguraci;


        int velikostPoleKonfiguraci = 0;
        MPI_Recv(&velikostPoleKonfiguraci, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_CONFIGURATION_ARRAY_SIZE, MPI_COMM_WORLD, &status);

        poleKonfiguraci = malloc(velikostPoleKonfiguraci * sizeof (Config));

        setVelikostPoleKonfiguraci(velikostPoleKonfiguraci);

        //printf("Jsem procesor %d a dostal jsem %d konfiguraci.\n", my_rank, lenght);

        //TODO: Nastavit hodnotu proměnné do mbg.c
        int pocetPrvku = 0;

        int j = 0;
        for (j = 0; j < lenght; j++) {
            Prvek* polePrvku;
            polePrvku = malloc(pocetVrcholu * sizeof (Prvek));

            int pocetBarev = 0;

            //Nejprve přijmeme počet barev
            MPI_Recv(&pocetBarev, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_COLORS, MPI_COMM_WORLD, &status);
            //printf("Počet barev: %i\n", pocetBarev);

            //Přijmeme počet prvků v konfiguraci
            MPI_Recv(&pocetPrvku, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_ITEMS, MPI_COMM_WORLD, &status);
            //printf("Počet prvku: %i\n", pocetPrvku);

            //Přijmeme pole prvků
            MPI_Recv(polePrvku, pocetPrvku * sizeof (Prvek), MPI_BYTE, i, MESSAGE_JOB_REQUIRE_CONFIGURATION_ARRAY, MPI_COMM_WORLD, &status);
            //printf("Přijmul jsem pole prvku: %d, %d\n", polePrvku[1].barva, polePrvku[1].vrchol);

            /*
             * Nastavíme proměnné do konfigurací
             */
            poleKonfiguraci[j].array = polePrvku;
            poleKonfiguraci[j].pocetBarev = pocetBarev;

        }

        /*
         * Odešleme konfigurace
         */
        setConfiguration(poleKonfiguraci, lenght, pocetPrvku);

        /*
         * Nyní řešíme zásobník
         */
        int zasobnikSize = 0;

        //Přijme počet vrcholů v zásobníku
        MPI_Recv(&zasobnikSize, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_STACK_SIZE, MPI_COMM_WORLD, &status);
        //printf("XXXXXXXXXPočet prvků v zásobníku: %d\n", zasobnikSize);

        int vrcholZasobniku = 0;

        //Přijme vrchol zásobníku
        MPI_Recv(&vrcholZasobniku, 1, MPI_INT, i, MESSAGE_JOB_REQUIRE_STACK_TOP, MPI_COMM_WORLD, &status);
        //printf("XXXXXXXXXVrchol zásobníku: %d\n", vrcholZasobniku);

        int* stackArray = malloc(zasobnikSize * sizeof (int));

        //Přijme vrchol zásobníku
        MPI_Recv(stackArray, zasobnikSize, MPI_INT, i, MESSAGE_JOB_REQUIRE_STACK_ARRAY, MPI_COMM_WORLD, &status);


        memoryFreeStack(s); //uvolníme předchozí zásobník
        s->array = stackArray;
        s->size = zasobnikSize;
        s->top = vrcholZasobniku;

        //printf("Před přijetím diagonály.\n");
        MPI_Recv(diag, pocetVrcholu, MPI_INT, i, MESSAGE_JOB_REQUIRE_DIAG, MPI_COMM_WORLD, &status);

        my_color = PESEK_BLACK;

    }
}

/*
 * Odpovíme na případný požadavek na práci
 */
void answerJobRequests(Stack* s, int pocetVrcholu) {
    MPI_Request request;

    int source;
    int flag;
    int message = -1;

    // kontrolujeme prichozi pozadavky na praci od vsech procesoru
    for (source = 0; source < processSum; source++) {

        // necekame praci sami od sebe
        if (source != my_rank) {
            /* checking if message has arrived */
            MPI_Iprobe(source, MESSAGE_JOB_REQUIRE, MPI_COMM_WORLD, &flag, &status);
            //printf("Testovani pozadavku na praci. Flag: %d, procesor: %d\n", flag, source);
            if (flag) {

                // ziskame pocet konfiguraci ktere muzeme predat
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
                    int velikostPole = getVelikostPoleKonfiguraci();

                    //printf("Jsem procesor: %d. Celkem konfiguraci: %d, odesilam pocet konfiguraci: %d, nechavam si: %d\n", my_rank, pocetKonfiguraci, konfiguraceKOdeslani, novyPocetKonfiguraci);

                    MPI_Send(&konfiguraceKOdeslani, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);
                    MPI_Send(&velikostPole, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_CONFIGURATION_ARRAY_SIZE, MPI_COMM_WORLD);

                    // posleme vsechny konfigurace
                    int l = 0;
                    for (l = 0; l < konfiguraceKOdeslani; l++) {
                        int odesilanaKonfigurace;
                        int pocetBarev;
                        int sizeOfPocetPrvku;
                        Prvek* array;

                        // rozdelime pocet konfiguraci na dve poloviny
                        odesilanaKonfigurace = novyPocetKonfiguraci + l;
                        pocetBarev = getPocetBarev(odesilanaKonfigurace);

                        array = getKonfigurace(odesilanaKonfigurace);
                        sizeOfPocetPrvku = getPocetPrvku();
                        MPI_Send(&pocetBarev, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_COLORS, MPI_COMM_WORLD);
                        MPI_Send(&sizeOfPocetPrvku, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ITEMS, MPI_COMM_WORLD);
                        //printf("Jsem procesor %d a odesilam konfiguraci %d prosesoru %d.\n", my_rank, odesilanaKonfigurace, source);
                        MPI_Isend(array, (sizeOfPocetPrvku * sizeof (Prvek)), MPI_BYTE, source, MESSAGE_JOB_REQUIRE_CONFIGURATION_ARRAY, MPI_COMM_WORLD, &request);
                        do {
                            MPI_Test(&request, &flag, &status);
                        } while (!flag);
                    }

                    // nyni posleme zasobnik neprohledanych vrcholu
                    int zasobnikTop;
                    int zasobnikSize;
                    int* zasobnikArray;

                    zasobnikTop = s->top;
                    zasobnikSize = s->size;
                    zasobnikArray = s->array;

                    MPI_Send(&zasobnikSize, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_STACK_SIZE, MPI_COMM_WORLD);
                    MPI_Send(&zasobnikTop, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_STACK_TOP, MPI_COMM_WORLD);
                    MPI_Isend(zasobnikArray, zasobnikSize, MPI_INT, source, MESSAGE_JOB_REQUIRE_STACK_ARRAY, MPI_COMM_WORLD, &request);
                    do {
                        MPI_Test(&request, &flag, &status);
                    } while (!flag);

                    MPI_Isend(diag, pocetVrcholu, MPI_INT, source, MESSAGE_JOB_REQUIRE_DIAG, MPI_COMM_WORLD, &request);
                    do {
                        MPI_Test(&request, &flag, &status);
                    } while (!flag);

                    if (source < my_rank) {
                        my_color = PESEK_BROWN;
                    }

                    // zmensime svuj pocet konfiguraci
                    setPocetKonfiguraci(novyPocetKonfiguraci);

                } else {

                    // odpovez ze nemam praci

                    //TODO: Upravit nothing
                    int nothing = NO_JOB;
                    MPI_Send(&nothing, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);
                    int konfiguraceKOdeslani = 0;
                    //MPI_Send(&konfiguraceKOdeslani, 1, MPI_INT, source, MESSAGE_JOB_REQUIRE_ANSWER, MPI_COMM_WORLD);

                    //printf("Odpovedel jsem ze namam praci k predani (Kdo: %d -> Komu: %d)\n", my_rank, source);

                }

            }
        }
    }
}

/*
 Funkce pro obsluhu peska na procesorech s ID vetsim nez 0
 */
void pesekOstatni() {
    if (my_rank > 0) {
        MPI_Request request;

        int pesek_hodnota = -1;
        int flag = 0;
        //Ověříme, zda nedorazil pešek
        MPI_Iprobe(my_rank - 1, PESEK, MPI_COMM_WORLD, &flag, &status);

        if (flag) {
            MPI_Recv(&pesek_hodnota, 1, MPI_INT, my_rank - 1, PESEK, MPI_COMM_WORLD, &status);

            //printf("Přijal jsem peška: %d, procesor: %d\n", pesek_hodnota, my_rank);

            if (my_color == PESEK_BLACK) {
                pesek_hodnota = PESEK_BLACK;
            }
            MPI_Isend(&pesek_hodnota, 1, MPI_INT, (my_rank + 1) % processSum, PESEK, MPI_COMM_WORLD, &request);

            my_color = PESEK_WHITE;
        }

        flag = 0;

        //Ověříme, zda nedorazil ukončovací pešek
        MPI_Iprobe(my_rank - 1, PESEK_FINAL, MPI_COMM_WORLD, &flag, &status);

        if (flag) {
            MPI_Recv(&pesek_hodnota, 1, MPI_INT, my_rank - 1, PESEK_FINAL, MPI_COMM_WORLD, &status);
            //printf("Ukončovací pešek! procesor: %d\n", my_rank);

            if (my_rank != (processSum - 1)) {
                MPI_Send(&pesek_hodnota, 1, MPI_INT, (my_rank + 1) % processSum, PESEK_FINAL, MPI_COMM_WORLD);
            }

            algoritmusUkoncen = 1;
        }
    }
}

/*
 Funkce pro obsluhu peska na procesoru 0
 */
void pesekRoot() {
    if (my_rank == 0) {
        MPI_Request request;

        //Zkusíme přijmout peška
        if (pesek_sent == 1) {
            int flag = 0;
            int pesek_hodnota = 999;
            //Ověříme, zda nedorazil pešek
            MPI_Iprobe(processSum - 1, PESEK, MPI_COMM_WORLD, &flag, &status);

            if (flag) {
                MPI_Recv(&pesek_hodnota, 1, MPI_INT, processSum - 1, PESEK, MPI_COMM_WORLD, &status);
                pesek_sent = 0;

                //printf("Přijal jsem peška: %d, procesor: %d\n", pesek_hodnota, my_rank);
            }

            // Odesíláme ukončovacího peška
            if (pesek_hodnota == PESEK_WHITE) {
                int final = PESEK_FINAL;
                //printf("Posílám ukončovacího peška\n");

                MPI_Send(&final, 1, MPI_INT, my_rank + 1, PESEK_FINAL, MPI_COMM_WORLD);

                algoritmusUkoncen = 1;
            }
        }
        //Konec přijímání peška

        if (algoritmusUkoncen == 0) {
            //Odesílání peška
            pokusyPrijetiPrace++;
            if (pokusyPrijetiPrace % processSum == 0) {

                //Jsem procesor 0 a odesílám peška

                if (pesek_sent == 0) {
                    int pesek_color = PESEK_WHITE;
                    MPI_Isend(&pesek_color, 1, MPI_INT, my_rank + 1, PESEK, MPI_COMM_WORLD, &request);
                    pesek_sent = 1;
                    //printf("Odeslal jsem peška. 0\n");
                }

            }
        }
    }
}
