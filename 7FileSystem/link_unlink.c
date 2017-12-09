#ifndef LINK_UNLINK_H
#define LINK_UNLINK_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "util.c"
#include "mkdir_creat_rmdir.c"


int my_link(char *old_file,char *new_file){
	int oino, odev, nion, ndev, pino;
	MINODE *omip, *pmip;
	char *ndir, *nbase, tempdir[128], tempbase[128];
	//check paramters
	if(!old_file){
		printf("link old_file is NULL\n");
		return -1;
	}
	if(!new_file){
		printf("link new_file is NULL\n");
		return -1;
	}
	if(old_file[0]=='/'){
		odev = root->dev;
	}
	else{
		odev = running->cwd->dev;
	}
	if(new_file[0]=='/'){
		ndev = root->dev;
	}
	else{
		ndev = running->cwd->dev;
	}
	//(1) verify old file exists and is not a DIR.
	oino = getino(&odev, old_file);
	if(oino == 0){
		printf("link old_file doesn't exist\n");
		return -1;
	}
	omip = iget(odev, oino);
	//check file type(cannot be DIR)
	if(S_ISDIR(omip->INODE.i_mode)){
		printf("In link, %s is a dir\n",old_file);
		iput(omip);
		return -1;
	}
	
	nion = getino(&ndev, new_file);
	if(nion!=0){
		printf("In link, %s already exists\n",new_file);
		iput(omip);
		return -1;
	}	
	//(2) must be able to create new file
	//get dirname and basename
	strcpy(tempdir,new_file);
	ndir = dirname(tempdir);
	strcpy(tempbase,new_file);
	nbase = basename(tempbase);
	
	pino = getino(&ndev, ndir);
	if(ndev!=odev){
		printf("In link, odev %d != ndev %d\n",odev,ndev);
		iput(omip);
		return -1;
	}
	
    pmip = iget(ndev,pino);
	//(3) creat entry in new parent DIR with same inode number of old file
	printf("in my_link before enter_child\n");
	enter_child(pmip, omip->ino, nbase);
	//(4) 
	omip->INODE.i_links_count++;
	omip->dirty = 1;
	iput(omip);
	iput(pmip);
	return 0;
}

int my_unlink(char *pathname){
	int dev,ino,pino,i;
	MINODE *mip, *pmip;
	char *dir, *base, tempdir[128], tempbase[128];
	//check parameters
	if(!pathname){
		printf("In unlink %s is NULL\n",pathname);
		return -1;
	}
	if(pathname[0]=='/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}
	//(1) get filename's minode
	ino = getino(&dev, pathname);	
	if(ino==0){
		printf("In unlink %s doesn't exist\n",pathname);
		return -1;
	}
	
	mip = iget(dev, ino);
	if(!S_ISREG(mip->INODE.i_mode) && !S_ISLNK(mip->INODE.i_mode)){
		printf("In unlink, %s is not a regular file or symlink\n",pathname);
		iput(mip);
		return -1;
	}
	//(2) remove nae entry from parent DIR's data block:
	//get dirname and basename
	strcpy(tempdir,pathname);
	dir = dirname(tempdir);
	strcpy(tempbase,pathname);
	base = basename(tempbase);

	//remove base from parent
    pino = getino(&dev,dir);
	pmip = iget(dev,pino);
	rm_child(pmip, base);
	pmip->dirty = 1;
	iput(pmip);
	//(3) -(5)
	if(mip->INODE.i_links_count > 0){
		mip->dirty = 1;
	}
	if(!S_ISLNK(mip->INODE.i_mode)){
	    for(i = 0; i < 12; i++){
		    if(mip->INODE.i_block[i] == 0)break;
		    bdealloc(dev,mip->INODE.i_block[i]);
	    }
	}
	idealloc(mip->dev, mip->ino);
	mip->dirty = 1;
	iput(mip);	
}


#endif