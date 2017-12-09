#ifndef CHOMD_H
#define CHOMD_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include "util.c"
#include "allocate_deallocate.c"

int my_chmod(char *mode, char *pathname){
	int dev, ino;
	MINODE* mip;
	INODE* ip;
	long int new_mode;
	//check input absolute or 相对
	if(pathname[0]=='/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}
	ino = getino(&dev, pathname);
	if(ino == 0){
		printf("In chmod, pathname = %s doesn't exist\n",pathname);
		return -1;
	}
	mip = iget(dev, ino);
	ip = &mip->INODE;
	new_mode = strtol(mode, NULL, 8);
	ip->i_mode = (ip->i_mode & 0xF000) | new_mode;
	mip->dirty = 1;
    iput(mip);
    return 0;
}

int my_touch(char *pathname){
	int dev;
	if(pathname[0]=='/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}
	int ino = getino(&dev, pathname);
	if(ino == 0){
		printf("no such file\n");
		return -1;
	}
	else{
		MINODE *mip=iget(dev, ino);
		mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
		mip->dirty = 1;
		iput(mip);
		return 0;
	}
}



#endif