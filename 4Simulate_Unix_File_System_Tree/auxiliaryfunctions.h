#ifndef _auxiliaryfunctions_h_
#define _auxiliaryfunctions_h_ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <libgen.h>

typedef struct node{
	char name[64];
	char node_type;
	struct node *childPtr;
	struct node *siblingPtr;
	struct node *parentPtr;
}NODE;


NODE *root, *cwd;/* root and CWD pointers */
char line[128];/* user input line */
char command[16], pathname[64];/* user inputs */
char DirName[64], BaseName[64];/* string holders */

int checkEmpty(NODE *target);
void traverseChild(NODE *parent);
NODE *levelSearch(NODE* cur, char name[64], char type);
NODE *iterativeLevelSearch(NODE *cur, char directoryName[64], char type);
void rpwd(NODE *P);
void delete(NODE *target);
int countToken(char *dir);
void printPath(char *dir);
void insert(NODE *parent, char NewName[64], char NewType);
char* getParentPath(NODE *cur);
void savePreOrder(NODE *ROOT,FILE *fp);

#endif










