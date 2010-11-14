/*
 * File:   main.c
 * Author: Martin Venuš, Jaroslav Líbal
 *
 * Created on 11. říjen 2010, 14:53
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "loadFile.h"
#include "dfs.h"

/*
 * Globální proměnné pro práci s MPI
 */
int my_rank;
int root = 0;
int processSum = 0;

/*
 * Globální proměnné pro práci s grafem
 */
int** maticeSousednosti;

/*
 * Výchozí velikost zásobníku (při vytvoření)
 */
#define INIT_STACK_SIZE 50

/*
 * Definice jednotlivých tagů při posílání zpráv
 */
#define MESSAGE_MATRIX 0

/*
 * @param argc počet argumentů zadaných při spuštění programu
 * @param argv argumenty předané při spuštění programu
 */
int main(int argc, char** argv) {

    /*
     * Inicializace a nastavení knihovny MPI
     */
    MPI_Status status;

    /* start up MPI */
    MPI_Init(&argc, &argv);

    /* find out process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &processSum);
    /*
     * Inicializace dokončena
     */

    int pocetVrcholu;


    if (my_rank == root) {

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

        maticeSousednosti = loadFile(inputFile);

        pocetVrcholu = getPocetVrcholu();

        int k = 0;
        for (k = 1; k < processSum; k++) {
            int l = 0;
            for (l = 0; l < pocetVrcholu; l++) {
                MPI_Send(maticeSousednosti[l], pocetVrcholu, MPI_INT, k, MESSAGE_MATRIX, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Bcast(&pocetVrcholu, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank > 0) {
        /*
         * Alokace paměti pro matici sousednosti
         */
        maticeSousednosti = malloc(pocetVrcholu * sizeof (int *));

        int i = 0;
        for (i = 0; i < pocetVrcholu; i++) {

            maticeSousednosti[i] = malloc(pocetVrcholu * sizeof (int));
        }

        int l = 0;
        for (l = 0; l < pocetVrcholu; l++) {
            MPI_Recv(maticeSousednosti[l], pocetVrcholu, MPI_INT, root, MESSAGE_MATRIX, MPI_COMM_WORLD, &status);
        }

        //printf("Matice(1): %d\n", maticeSousednosti[pocetVrcholu - 1][pocetVrcholu - 1 - 1]);

    }

    Stack s;

    init(&s, INIT_STACK_SIZE);

    //stackPrint(&S);

    DFS_analyse(&s, maticeSousednosti, pocetVrcholu);

    /*
     * Nalezneme nejlepší řešení a zapíšeme ho na diagonálu matice sousednosti
     */
    findBestColouring();
    setBestSolutionToMatrix(pocetVrcholu);
    printBestSolution(pocetVrcholu);

    /*
     * Uvolníme paměť alokovanou pro jednotlivé struktury
     */
    memoryFreeMatrix(maticeSousednosti, pocetVrcholu);

    memoryFreeStack(&s);
    memoryFreeConfigurationArray();

    /* shut down MPI */
    MPI_Finalize();

    return (EXIT_SUCCESS);
}

