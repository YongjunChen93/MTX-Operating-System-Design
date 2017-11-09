#ifndef GLOBAL_C_H
#define GLOBAL_C_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "type.h"

MINODE minode[NMINODE];
MINODE *root;
MOUNT *rootfilesys;
PROC   proc[NPROC], *running;
MOUNT mounttab[NMOUNT];
OFT oft[NOFT];

char names[64][128], *name[64];
int fd, dev, n;
int nblocks, ninodes, bmap, imap, inode_start;
char line[256], cmd[32], pathname[256], parameter[256];
char *CMD[] = {"ls","cd","pwd","mkdir","creat","rmdir","quit"};
#endif