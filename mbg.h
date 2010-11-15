/*
 * File:   mbg.h
 * Author: Martin Venuš, Jaroslav Líbal
 *
 * Created on 12. říjen 2010, 22:53
 */

#ifndef MBG_H
#define	MBG_H

typedef struct {
    int vrchol;
    int barva;
} Prvek;

typedef struct {
    Prvek* array;
    int pocetBarev;
} Config;

void coloring(int vrchol, int pocetVrcholu);
void memoryFreeConfigurationArray();
Prvek* copyArray(Prvek* sourceArray, int pocetVrcholu, int pocetPrvku);
void setMaticeSousednosti(int** matrix);
int sousedi(int vrchol1, int vrchol2);
void addVrchol(int konfigurace, int vrchol, int barva, int pocetVrcholu);
void findBestColouring();
void setBestSolutionToMatrix(int pocetVrcholu);
void printBestSolution(int pocetVrcholu);
int getPocetKonfiguraci();

#endif	/* MBG_H */

