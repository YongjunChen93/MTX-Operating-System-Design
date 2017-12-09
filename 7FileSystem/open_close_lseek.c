#ifndef OPEN_CLOSE_LSEEK_H
#define OPEN_CLOSE_LSEEK_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <sys/stat.h>

#include "util.c"

/*
Open File Data Structures:

    running
      |                                                  
      |                                                    ||****************** 
    PROC[ ]              OFT[ ]              MINODE[ ]     ||      Disk dev
  ===========    |---> ===========    |--> ============    || ===============
  |ProcPtr  |    |     |mode     |    |    |  INODE   |    || |      INODE   
  |pid, ppid|    |     |refCount |    |    | -------  |    || =============== 
  |uid      |    |     |minodePtr|---->    | dev,ino  |    || 
  |cwd      |    |     |offset   |         | refCount |    ||******************
  |         |    |     ====|======         | dirty    |
  |  fd[10] |    |         |               | mounted  |         
  | ------  |    |         |               ============
0 |   ----->|--->|         |
  | ------  |              |   
1 |         |              |
  | ------  |             --------------------------------
2 |         |             |0123456.............
  | ------  |             --------------------------------    
  ===========        logical view of file: a sequence of bytes
                          
 */ 

//flag: 0|1|2|3|4 for R|W|RW|APPEND
int my_open(char *pathname, int flags){
	int dev, ino, i;
	MINODE *mip;
	OFT *oftp;
	INODE *ip;
	//check input
	printf("this is my open\n");
	if(flags < 0 || flags > 4){
		printf("In open flags = %d error \n", flags);
		return -1;
	}
	if(!pathname || pathname[0]==0){
		printf("In open pathname doesn't exist\n");
		return -1;
	}
	//1. get ino
	ino = getino(&dev, pathname);
	if(ino == 0){
		if(flags!=4){
			printf("%s doesn't exist\n", pathname);
			return -1;
		}
		my_creat(pathname);
		ino = getino(&dev, pathname);
	}
	mip = iget(dev, ino);
	ip = &(mip->INODE);

	//2. check file INODE's access perimission
	//should be regular file and perimission OK
	if(!S_ISREG(mip->INODE.i_mode)){
		printf("Error: it's not a regular file.\n");
		iput(mip);
		return -1;
	}
	//not equal and it's not super user
	if((running->uid!=mip->INODE.i_uid)&&(running->uid!=0)){
		printf("No permission\n");
		return -1;
	}
	//2.2 check for icompatible open modes
	for(i=0;i<NOFT;i++){
		if(oft[i].refCount == 0)
			continue;
		if(oft[i].refCount!=0 && oft[i].mptr->ino == ino){
			if(flags!=0){
				printf("file %s has been opened\n",pathname);
				iput(mip);
				return -1;
			}
			else{
				if(oft[i].mode!=0){
					printf("file %s has been opened\n", pathname);
					iput(mip);
					return -1;
				}
			}
		}
	}
	//3. allocate an openTable entry
	oftp = (OFT *)malloc(sizeof(OFT));
	oftp->mode = flags;
	oftp->refCount = 1;
	oftp->mptr = mip;
	if(flags == 4){
		oftp->offset = ip->i_size;
	}
	else{
		oftp->offset = 0;
	}
	//4. search for a FREE fd[] entry with the lowest index fd inPROC;
	for(i=0; i<NFD; i++){
		if(!running->fd[i]){
			running->fd[i] = oftp;
			break;
		}
	}
	//4.2 update INODE's time 
	if(flags == 0){
		mip->INODE.i_atime = time(0L);
	}
	else{
		mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
	}
	//5. unlock minode
	mip->dirty = 1;
	printf("fd of opened file:%d \n",i);
	return i;
}

int my_close(int fd){
	int i;
	OFT *oftp;
	MINODE *mip;
	//(1) check fd is valid
	if(validfd(fd)<0){
		printf("In my close, %d out of range\n",fd);
		return -1;
	}
	//(2) ,(4) 
	if(running->fd[fd] != 0){
		oftp = running->fd[fd];
		mip = oftp->mptr;
		if(--oftp->refCount == 0){
			iput(mip);
		}
	}
	//(5)
	running->fd[fd] = 0;
	//(6)
	return 0;

}

int validfd(int fd){
	if(fd<0 || fd >= NFD){
		return -1;
	}
	return 0;
}

int mylseek(int fd, int position)
{
  	//From fd, find the OFT entry. running->fd[fd]
  	//change OFT entry's offset to position but make sure NOT to over run either end of the file.
	// should range in [0, file_size]
	//if position > file_size
	if(running->fd[fd]->mptr->INODE.i_size>=position) {
		if(position>=0)
			running->fd[fd]->offset=position;
		else 
			running->fd[fd]->offset=0;
	}
	else{
		running->fd[fd]->offset=running->fd[fd]->mptr->INODE.i_size;
	} 
	//return originalPosition
	printf("lseek offset: %d\n",running->fd[fd]->offset);
	return running->fd[fd]->offset;
}

#endif