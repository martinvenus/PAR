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
 * Výchozí velikost zásobníku (při vytvoření)
 */
#define INIT_STACK_SIZE 50

/*
 * @param argc počet argumentů zadaných při spuštění programu
 * @param argv argumenty předané při spuštění programu
 */
int main(int argc, char** argv) {

    /*
     * Inicializace a nastavení knihovny MPI
     */
    int my_rank;
    int p;
    MPI_Status status;

    /* start up MPI */
    MPI_Init(&argc, &argv);

    /* find out process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    /*
     * Inicializace dokončena
     */

    int** maticeSousednosti;
    int pocetVrcholu;


    if (my_rank == 0) {

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
        for (k = 1; k < p; k++) {
            int l = 0;
            for (l = 0; l < pocetVrcholu; l++) {
                MPI_Send(maticeSousednosti[l], pocetVrcholu, MPI_INT, k, 0, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Bcast(&pocetVrcholu, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank > 0) {
        int matrix[pocetVrcholu][pocetVrcholu];

        printf("procesor %d přijímá\n", my_rank);

        int l = 0;
        for (l = 0; l < pocetVrcholu; l++) {
            MPI_Recv(&matrix[l], pocetVrcholu, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        }

        printf("Matice(1): %d\n", matrix[pocetVrcholu-1][pocetVrcholu-1-1]);

    }

    //printf("Počet vrcholů: %i\n", pocetVrcholu);

    /*

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
     */

    /* shut down MPI */
    MPI_Finalize();

    return (EXIT_SUCCESS);
}

