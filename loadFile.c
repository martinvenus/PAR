/*
 * File:   loadFile.c
 * Author: Martin Venuš, Jaroslav Líbal
 *
 * Created on 11. říjen 2010, 14:53
 */

#include <stdio.h>
#include <stdlib.h>
#include "loadFile.h"

/*
 * Velikost bufferu pro načtení počtu vrcholů matice ze souboru
 */
#define BUFFER_SIZE 50

/*
 * Počet vrcholů v načtené matici
 */
static int pocetVrcholu = 0;

/*
 * Funkce načte matici ze souboru do dvojrozměrného pole (dynamického)
 * @param file jméno souboru, ze kterého se načítá
 * @return dvojrozměrné pole s načtenou maticí
 */
int** loadFile(char* file) {
    FILE *f;
    f = fopen(file, "r");

    if (f == NULL) {
        printf("File cannot be opened for reading.\n");
        exit(1);
    } else {
        //printf("File was sucesfully opened.\n");
    }

    /*
     * Načtení počtu vrcholů ze souboru
     */
    char buffer[BUFFER_SIZE];

    fgets(buffer, BUFFER_SIZE, f);

    pocetVrcholu = atoi(buffer);


    int** maticeSousednosti;

    maticeSousednosti = malloc(pocetVrcholu * sizeof (int *));

    //printf("Pocet vrcholu: %i\n", pocetVrcholu);

    int pocet = 0;

    int i = 0;
    int j = 0;
    char znak = ' ';

    int cislo = -1;

    for (i = 0; i < pocetVrcholu; i++) {

        maticeSousednosti[i] = malloc(pocetVrcholu * sizeof (int));

        for (j = 0; j <= pocetVrcholu; j++) {
            znak = fgetc(f);
            cislo = znak - 48;

            //Načteme konec řádky a zahodíme - nepotřebujeme
            if (j == pocetVrcholu) {
                break;
            }

            maticeSousednosti[i][j] = cislo;
            // printf("%i", cislo);

            pocet++;
        }

        //printf("\n\n\n");
    }

    //printf("\nPocet: %i\n", pocet);


    //printf("\nPocet vrcholů: %i\n", pocetVrcholu);

    //Uzavření souboru
    fclose(f);

    return maticeSousednosti;

}

/*
 * Funkce vrátí počet vrcholů načtené matice
 * @return počet vrcholů
 */
int getPocetVrcholu() {
    return pocetVrcholu;
}

/*
 * Funkce uvolní paměť, která byla alokována pro matici sousednosti
 * @param matrix matice sousednosti
 * @param pocetVrcholu počet vrcholů (řádků=sloupců matice)
 */
void memoryFreeMatrix(int** matrix, int pocetVrcholu) {
    int i = 0;

    for (i = 0; i < pocetVrcholu; i++) {
        free(matrix[i]);
    }

    free(matrix);
}