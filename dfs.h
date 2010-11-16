/* 
 * File:   dfs.h
 * Author: Martin Venuš, Jaroslav Líbal
 *
 * Created on 11. říjen 2010, 19:03
 */

#ifndef DFS_H
#define	DFS_H

typedef struct
{
    int* array;
    int top;
    int size;
} Stack;


void push(Stack *S, int value);
int pop(Stack *S);
void init(Stack *S, int size);
int full(Stack *S);
int isEmpty(Stack *s);
void stackPrint(Stack *S);
void memoryFreeStack(Stack *s);
void DFS_analyse(Stack *s, int** m, int vrcholy);
void answerJobRequests(Stack* s, int pocetVrcholu);
void askForJob(int pocetVrcholu, Stack* s);



#endif	/* DFS_H */

