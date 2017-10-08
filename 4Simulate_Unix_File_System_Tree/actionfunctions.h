#ifndef _actionfunctions_h_
#define _actionfunctions_h_ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <libgen.h>

extern void initialize();//initialize / node of the file system tree
extern int findCmd(char *command);
extern void mkdir(char *pathname);
extern void rmdir(char *pathname);
extern void cd(char *pathname);
extern void ls(char *pathname);
extern void pwd();
extern void creat(char *pathname);
extern void rm(char *pathname);
extern void save();
extern void reload();
extern void menu();
extern void quit();
#endif 