/*
 * File:   mbg.c
 * Author: Martin Venuš, Jaroslav Líbal
 *
 * Created on 12. říjen 2010, 22:56
 */

#include <stdio.h>
#include <stdlib.h>
#include "mbg.h"

#define	NEW_COLOR -1

//TODO: Upravit na dynamickou relokaci paměti
int velikostPole = 1000;
int pocetKonfiguraci = 0;
int pocetPrvku = 0;
int pridanoKonfiguraci = 0;

int isCurrentConfigurationOver = 0;

int idBestSolution = -1;
int countColorBestSolution = -1;

Config* poleKonfiguraci;
extern int** maticeSousednosti;

extern int my_rank;

void coloring(int idNovehoVrcholu, int pocetVrcholu) {

    printf("Počet konfigurací: %d na procesoru: %d\n, velikost pole: %d", pocetKonfiguraci, my_rank, velikostPole);

    //printf("Debug - Obarvuji vrchol %i\n", idNovehoVrcholu);


    if (pocetKonfiguraci == 0) {

        //printf("\n\n\n\nNULA KONFIGURACI\n\n\n\n");

        poleKonfiguraci = malloc(velikostPole * sizeof (Config));

        //Přidáváme první vrchol
        Prvek novyVrchol;
        novyVrchol.vrchol = idNovehoVrcholu;
        novyVrchol.barva = 0;

        Prvek* polePrvku;

        polePrvku = malloc(pocetVrcholu * sizeof (Prvek));

        polePrvku[0] = novyVrchol;

        poleKonfiguraci[0].array = polePrvku;
        poleKonfiguraci[0].pocetBarev = 1;
        pocetKonfiguraci++;
        pocetPrvku++;

        //printf("Debug - vytvorena prvni konfigurace\n");

        return;

    }

    int i = 0;
    pridanoKonfiguraci = 0;
    printf("Debug - pocet konfiguraci: %i\n", pocetKonfiguraci);

    for (i = 0; i < pocetKonfiguraci; i++) {

        isCurrentConfigurationOver = 0;

        //printf("\n\n\nDebug - prochazim konfiguraci %i\n", i);

        int currentColor = 0; // zacinam barvit od nuly
        int notFound = 0; // indikator ze bylo nalezeno možné obarvení
        int noColor = 1;

        // projdu vsechny dostupne barvy
        while (currentColor < poleKonfiguraci[i].pocetBarev) {
            //printf("Zkouším barvu: %i\n", currentColor);
            notFound = 0;

            //printf("Debug - pocet barev v konfiguraci %i je %i\n", i, poleKonfiguraci[i].pocetBarev);

            int j = 0;
            for (j = 0; j < pocetPrvku; j++) {

                //Jestliže se jedná o sousední vrcholy a zároveň mají stejnou barvu
                if ((sousedi(poleKonfiguraci[i].array[j].vrchol, idNovehoVrcholu) == 1) && (poleKonfiguraci[i].array[j].barva == currentColor)) {
                    //pokračujeme, protože se nový vrchol současnou barvou nedá obarvit
                    notFound = 1;
                    //printf("Porovnavam vrcholy %i a %i a jejich barvy %i a %i\n", poleKonfiguraci[i].array[j], idNovehoVrcholu, poleKonfiguraci[i].array[j].barva, currentColor);
                    break;
                }

            }

            if (notFound == 0) {
                //printf("Nalezeno obarveni: %i.\n", currentColor);

                //TODO: Zde přidáme vrchol ke konfiguraci
                addVrchol(i, idNovehoVrcholu, currentColor, pocetVrcholu);

                noColor = 0;
            }

            //A pokračujeme s další barvou
            currentColor++;

        }

        //Nenalezli jsme žádnou možnost jakou barvou obarvit přidávaný vrchol -> musíme přidat novou barvu
        if (noColor == 1) {
            // printf("Nebylo nalezeno obarveni.\n");
            //Přidáme barvu a rovnou touto barvou obarvíme právě přidávaný vrchol a přidáme ho ke konfiguraci
            addVrchol(i, idNovehoVrcholu, NEW_COLOR, pocetVrcholu);
        }
    }

    pocetKonfiguraci += pridanoKonfiguraci;
    pocetPrvku++;

    //printf("Počet prvků: %i\n", pocetPrvku);
}

void addVrchol(int konfigurace, int vrchol, int barva, int pocetVrcholu) {

    //printf("Přidávám prvek. Pocet prvku: %i, vrchol: %i, barva: %i\n", pocetPrvku, vrchol, barva);
    //printf("Pocet konfiguraci: %i\n", pocetKonfiguraci);

    if (isCurrentConfigurationOver == 0) {
        //printf("Prvni vetev pridavani.\n");

        poleKonfiguraci[konfigurace].array[pocetPrvku].vrchol = vrchol;

        if (barva == NEW_COLOR) {
            poleKonfiguraci[konfigurace].array[pocetPrvku].barva = poleKonfiguraci[konfigurace].pocetBarev;
            poleKonfiguraci[konfigurace].pocetBarev++;
        } else {
            poleKonfiguraci[konfigurace].array[pocetPrvku].barva = barva;
        }

        //printf("Vrchol: %i, Barva: %i\n", poleKonfiguraci[konfigurace].array[pocetPrvku].vrchol, poleKonfiguraci[konfigurace].array[pocetPrvku].barva);

    } else {

        int oldConf = konfigurace;
        int newConf = pocetKonfiguraci + pridanoKonfiguraci;

        /*
         * Zvětšíme pole
         */
        if (newConf >= velikostPole){
            zvetsiPole();
        }

        //printf("Druha vetev pridavani. Pridano konfiguraci: %i\n", pridanoKonfiguraci);
        Prvek* newArray;
        newArray = copyArray(poleKonfiguraci[oldConf].array, pocetVrcholu, pocetPrvku);

        poleKonfiguraci[newConf].array = newArray;

        poleKonfiguraci[newConf].pocetBarev = poleKonfiguraci[oldConf].pocetBarev;

        poleKonfiguraci[newConf].array[pocetPrvku].vrchol = vrchol;
        poleKonfiguraci[newConf].array[pocetPrvku].barva = barva;

        //printf("Vrchol: %i, Barva: %i\n", poleKonfiguraci[newConf].array[pocetPrvku].vrchol, poleKonfiguraci[newConf].array[pocetPrvku].barva);

        pridanoKonfiguraci++;


    }


    //showData();

    isCurrentConfigurationOver = 1;
}

void zvetsiPole(){
        int novaVelikost = 2 * velikostPole;
        poleKonfiguraci = realloc(poleKonfiguraci, novaVelikost * sizeof (int));

        if (poleKonfiguraci == NULL) {
            printf("Memory reallocation failed for poleKonfiguraci.\n");
        } else {
            velikostPole = novaVelikost;
        }
    }

Prvek* copyArray(Prvek* sourceArray, int pocetVrcholu, int pocetPrvku) {

    Prvek* array;
    array = malloc(pocetVrcholu * sizeof (Prvek));

    int i = 0;
    for (i = 0; i < pocetPrvku; i++) {

        array[i] = sourceArray[i];
    }

    for (i = 0; i < pocetPrvku; i++) {

        //printf("Prochazim nove pole: %i, %i", array[i].vrchol, array[i].barva);
    }



    return array;
}

int sousedi(int vrchol1, int vrchol2) {
    if (maticeSousednosti[vrchol1][vrchol2] == 1) {

        return 1;
    }

    return 0;
}

/*
 * Funkce uvolní paměť, která byla alokována pro pole
 */
void memoryFreeConfigurationArray() {
    int i = 0;

    for (i = 0; i < pocetKonfiguraci; i++) {
        free(poleKonfiguraci[i].array);
    }

    free(poleKonfiguraci);
}

void showData() {
    int i = 0;
    for (i = 0; i < pocetKonfiguraci; i++) {
        //printf("\nDebug - Pocet barev v konfiguraci %i: %i\n", i, poleKonfiguraci[i].pocetBarev);

        int j = 0;
        for (j = 0; j < pocetPrvku; j++) {
            //printf("Debug - Vrchol %i, barva: %i\n", poleKonfiguraci[i].array[j].vrchol, poleKonfiguraci[i].array[j].barva);
        }
    }
}

void findBestColouring() {
    int best = -1;
    int bestPocetBarev = 9999;

    int i = 0;
    for (i = 0; i < pocetKonfiguraci; i++) {
        if (poleKonfiguraci[i].pocetBarev < bestPocetBarev) {
            best = i;
            bestPocetBarev = poleKonfiguraci[i].pocetBarev;
        }
    }

    idBestSolution = best;
    countColorBestSolution = bestPocetBarev;
}

void setBestSolutionToMatrix(int pocetVrcholu) {
    int i = 0;

    for (i = 0; i < pocetPrvku; i++) {
        maticeSousednosti[ poleKonfiguraci[idBestSolution].array[i].vrchol ][ poleKonfiguraci[idBestSolution].array[i].vrchol ] = poleKonfiguraci[idBestSolution].array[i].barva;
    }

}

void printBestSolution(int pocetVrcholu) {
    printf("Pocet pouzitych barev: %i\n", countColorBestSolution);

    int i = 0;
    int j = 0;

    for (i = 0; i < pocetVrcholu; i++) {
        for (j = 0; j < pocetVrcholu; j++) {
            printf("%i", maticeSousednosti[i][j]);
        }
        printf("\n");
    }
}

int getPocetKonfiguraci() {
    return pocetKonfiguraci;
}

void setPocetKonfiguraci(int pocet) {

    int i;
    for (i = pocet + 1; i < pocetKonfiguraci; i++) {
        free(poleKonfiguraci[i].array);
    }
    pocetKonfiguraci = pocet;

}

int getPocetBarev(int konfigurace) {
    return poleKonfiguraci[konfigurace].pocetBarev;
}

int getPocetPrvku() {
    return pocetPrvku;
}

int getVelikostPoleKonfiguraci() {
    return velikostPole;
}

void setVelikostPoleKonfiguraci(int size) {
    velikostPole = size;
}

Prvek* getKonfigurace(int konfigurace) {
    //printf("\nPozadavek na konfiguraci: %d z celkoveho poctu: %d\n", konfigurace+1, pocetKonfiguraci);
    //printf("Vypis konfigurace: %d, vrchol 0 ma barvu %d\n", konfigurace, poleKonfiguraci[konfigurace].array[0].barva);
    return poleKonfiguraci[konfigurace].array;
}

void setConfiguration(Config* pole, int pocetKonfiguraciArg, int pocetPrvkuArg) {
    printf("Prijimam konfigurace. Nahodny prvek(0): %d\n\n\n", pole[0].pocetBarev);
    poleKonfiguraci = pole;

    printf("Nastavuji pocet prvku: %d\n\n\n", pocetPrvkuArg);
    pocetPrvku = pocetPrvkuArg;

    pocetKonfiguraci = pocetKonfiguraciArg;
}