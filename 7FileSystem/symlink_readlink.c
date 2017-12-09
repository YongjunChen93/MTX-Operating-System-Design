#ifndef SYMLINK_READLINK_H
#define SYMLINK_READLINK_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "util.c"
#include "mkdir_creat_rmdir.c"

int my_symlink(char *old_file, char *new_file)
{
	int odev, ndev, oino, nino, bno;
	MINODE *nmip;
	char *cp;
	
	if(!old_file){
		printf("In symlink old_file = %s is NULL\n",old_file);
		return -1;
	}
	if(!new_file){
		printf("In symlink new_file = %s is NULL\n",new_file); 
		return -1;
	}
	if(old_file[0]=='/'){
		odev = root->dev;
	}
	else{
		odev = running->cwd->dev;
	}
	//(1) check old file mus exists and new file not yet exists
	oino = getino(&odev, old_file);
	if(oino==0){
		printf("In symlink, %s doesn't exist\n",old_file);
		return -1;
	}
	if(new_file[0]=='/'){
		ndev = root->dev;
	}
	else{
		ndev = running->cwd->dev;
	}
	nino = getino(&ndev, new_file);
	if(nino!=0){
		printf("In symlink, %s already exists\n",new_file);
		return -1;
	}
	//(2) same as creat new file 
	my_creat(new_file);		
	nino = getino(&ndev, new_file);
	nmip = iget(ndev,nino);
	//change new_file to SLINK type 0120000
	nmip->INODE.i_mode &= 0x0FFFF;
	nmip->INODE.i_mode |= 0xA000;	
	//(3) assume length of old_file name <= 60 chars,store old_file name in newfile's INODE.i_block[ ] area
	cp = (char *)(nmip->INODE.i_block);
	strcpy(cp,old_file);
	nmip->dirty = 1;
	iput(nmip);	
	return 0;
}

int my_readlink(char *file, char *buffer){
	printf("==================readlinke ++++++++++++++++++++++");
	int dev, ino, bno;
	MINODE *mip;
	char *cp;
	//check parameters
	if(!file){
		printf("In readlink %s is NULL\n",file);
		return -1;
	}
	if(file[0]=='/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}
	//(1) get file's INODE in memory; verify it's a LNK file
	ino = getino(&dev, file);
	if(ino==0){
		printf("In readlink %s doesn't exist\n",file);
		return -1;
	}
	mip = iget(dev,ino);
	
	//verify it's a SLINK file
	if((mip->INODE.i_mode & 0xA000) != 0xA000){
		printf("In readlink %s is not a symlink\n",file);
		iput(mip);
		return -1;
	}
	//(2) copy target filenams from INODE i_block[] into buffer
	cp = (char *)(mip ->INODE.i_block);
	strcpy(buffer,cp);
	iput(mip);
	printf("length of the target file: %d\n",strlen(cp));
	//(3) return file size
	return strlen(cp);	
}

#endif