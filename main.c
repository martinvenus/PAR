/* 
 * File:   main.c
 * Author: Martin Venuš, Jaroslav Líbal
 *
 * Created on 11. říjen 2010, 14:53
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loadFile.h"
#include "dfs.h"

/*
 * Výchozí velikost zásobníku (při vytvoření)
 */
#define INIT_STACK_SIZE 50

/*
 * @param argc počet argumentů zadaných při spuštění programu
 * @param argv argumenty předané při spuštění programu
 */
int main(int argc, char** argv) {

    char inputFile[255] = "";

    int i = 0;
    int j = 0;

    //Načtení proměnných z příkazového řádku
    while (argv[i] != NULL) {
        //printf("Argument: %s\n", argv[i]);
        if (strcmp(argv[i], "-f") == 0) {
            strcpy(inputFile, argv[i + 1]);
        }
        i++;
    }

    int** maticeSousednosti;
    maticeSousednosti = loadFile(inputFile);

    int pocetVrcholu = getPocetVrcholu();

    Stack s;

    init(&s, INIT_STACK_SIZE);

    //stackPrint(&S);

    setMaticeSousednosti(maticeSousednosti);
    DFS_analyse(&s, maticeSousednosti, pocetVrcholu);

    //Uvolníme paměť alokovanou pro matici
    //free(DFS);
    
    findBestColouring();
    setBestSolutionToMatrix(pocetVrcholu);

    memoryFreeMatrix(maticeSousednosti, pocetVrcholu);

    //showData();

    memoryFreeStack(&s);
    memoryFreeArray();

    return (EXIT_SUCCESS);
}

