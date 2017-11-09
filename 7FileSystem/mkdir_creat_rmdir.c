#ifndef MKDIR_CREAT_RMDIR_H
#define MKDIR_CREAT_RMDIR_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "util.c"
#include "allocate_deallocate.c"

MINODE * get_pmip(char *pathname, char *dir, char *base){
	int pino,dev;
	MINODE* pmip;
	
	if(!pathname){
		printf("mkdir pathname is NULL\n");
		return NULL;
	}
	if(pathname[0]=='/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}
	
	pino = getino(&dev, dir);
	pmip = iget(dev, pino);
	if(!S_ISDIR(pmip->INODE.i_mode)){
		printf("%s is not a dir\n",dir);
		iput(pmip);
		return NULL;
	}
	
	if(search(pmip, base) != 0){
		printf("%s already exists\n",base);
		iput(pmip);
		return NULL;
	}
	return pmip;
}

int my_mkdir(char *pathname){
	MINODE* pmip;
	char *dir, *base;	
	char tempdir[128], tempbase[128];	
	strcpy(tempdir,pathname);
	dir = dirname(tempdir);
	strcpy(tempbase,pathname);
	base = basename(tempbase);
	printf("In mkdir, dirname = %s, basename = %s\n",dir,base);	
	pmip = get_pmip(pathname, dir, base);
	if(NULL==pmip) return -1;
	kmkdir(pmip, base);
	pmip->INODE.i_links_count++;
	pmip->dirty = 1;
	iput(pmip);
	return 0;
}

int need_len(int name_len){
	return 4*((8+name_len+3)/4);
}

void kmkdir(MINODE* pmip, char *base){
	int ino, blk, i;
	MINODE *mip;
	INODE *inp;
	char sbuf[BLKSIZE], *cp;
	DIR *dp;
    int dev;

    dev = pmip->dev;	
	ino = ialloc(dev);
	blk = balloc(dev);
	mip = iget(dev,ino);

	inp = &mip->INODE;
	inp->i_mode = DIR_MODE;
	inp->i_uid  = running->uid;	  // Owner uid 
    inp->i_gid  = running->gid;	  // Group Id
    inp->i_size = BLKSIZE;	      // Size in bytes 
    inp->i_links_count = 2;	              // . and ..
    inp->i_atime = time(0L);         // Set last access to current time
    inp->i_ctime = time(0L);         // Set creation to current time
    inp->i_mtime = time(0L);         // Set last modified to current time
    inp->i_blocks = BLKSIZE / 512; // # of 512-byte blocks reserved for this inode 
    inp->i_block[0] = blk;              // Has 1 data block for . and .. dir entries

	for(i=1;i<15;i++){
		inp->i_block[i] = 0;
	}
	mip->dirty = 1;
	iput(mip);
	get_block(dev, blk, sbuf);
	cp = sbuf;
	dp = (DIR*)cp;
	dp->inode = ino;
	dp->rec_len = need_len(1);
	dp->name_len = 1;
	dp->file_type = (u8)EXT2_FT_DIR;
	strcpy(dp->name, ".");
	
	cp += dp->rec_len;
	dp = (DIR*)cp;
	dp->inode = pmip->ino;
	dp->rec_len = BLKSIZE - 12;
	dp->name_len = 2;
	dp->file_type = (u8)EXT2_FT_DIR;
	strcpy(dp->name, "..");
	put_block(dev, blk, sbuf);
	if(enter_child(pmip, ino, base)!=0){
		printf("enter_child failed, cannot find a space\n");
		return;
	}
}

int enter_child(MINODE *pmip, int ino, char *base){
	int nlen, ideal_len, remain, i, blk, dev;
	INODE *pip;
	char sbuf[BLKSIZE], *cp;
	DIR *dp;
	dev = pmip->dev;	
	pip = &pmip->INODE;
	nlen = need_len(strlen(base));
	for(i=0;i<12;i++){
		printf("i = %d\n",i);
		printf("pip->i_block[%d]==%d\n",i,pip->i_block[i]);
		if(pip->i_block[i]==0){
			blk = balloc(dev);
			if(blk<=0){
				printf("enter_child allocate block error\n");
				return -1;
			}
			pip->i_block[i] = blk;
			pip->i_size += BLKSIZE;
			pmip->dirty = 1;
			get_block(dev, pip->i_block[i], sbuf);
			dp->inode = ino;
            dp->rec_len = BLKSIZE;
            dp->name_len = strlen(base);
            dp->file_type = (u8)EXT2_FT_DIR;
            strcpy(dp->name, base);
			put_block(dev, pip->i_block[i], sbuf);
			return 0;
		}		
		
		get_block(dev, pip->i_block[i], sbuf);
		printf("after get_block\n");
		cp = sbuf;
		dp = (DIR *)cp;
		if(dp->inode==0){
			printf("dp->inode==0\n");
			dp->inode = ino;
			dp->rec_len = BLKSIZE;
			dp->name_len = strlen(base);
			dp->file_type = (u8)EXT2_FT_DIR;
			strcpy(dp->name, base);
			put_block(dev, pip->i_block[i], sbuf);
			return 0;
		}
		
		//get last entry in block
		while (cp + dp->rec_len < sbuf + BLKSIZE){
			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
		printf("after get last entry in block\n");
		ideal_len = need_len(dp->name_len);
		remain = dp->rec_len-ideal_len;
		printf("remain = %d,nlen = %d\n",remain,nlen);
		if(remain >= nlen){
			dp->rec_len = ideal_len;
			cp += dp->rec_len;
			dp = (DIR *)cp;
			dp->inode = ino;
	        dp->rec_len = remain;
	        dp->name_len = strlen(base);
	        dp->file_type = (u8)EXT2_FT_DIR;
			printf("before strcpy\n");
	        strcpy(dp->name, base);
			printf("after strcpy\n");
			put_block(dev, pip->i_block[i], sbuf);
			printf("after put_block\n");
			return 0;
		}
	}
	return -1;
}

void my_creat(char *pathname){
	MINODE* pmip;
	char *dir, *base;
	char tempdir[128], tempbase[128];
	
	strcpy(tempdir,pathname);
	dir = dirname(tempdir);
	strcpy(tempbase,pathname);
	base = basename(tempbase);
	printf("In creat, dirname = %s, basename = %s\n",dir,base);
	
	pmip = get_pmip(pathname, dir, base);
	if(NULL==pmip) return -1;
	
	kcreat(pmip, base);
	pmip->dirty = 1;
	iput(pmip);	
}

void kcreat(MINODE* pmip, char *base){
	int ino, blk, i;
	MINODE *mip;
	INODE *inp;
	char sbuf[BLKSIZE], *cp;
	DIR *dp;
    int dev;

    dev = pmip->dev;	
	ino = ialloc(dev);
	mip = iget(dev,ino);
	inp = &mip->INODE;
	inp->i_mode = FILE_MODE;
	inp->i_uid  = running->uid;	  // Owner uid 
    inp->i_gid  = running->gid;	  // Group Id
    inp->i_size = 0;	      // Size in bytes 
    inp->i_links_count = 2;	              // . and ..
    inp->i_atime = time(0L);         // Set last access to current time
    inp->i_ctime = time(0L);         // Set creation to current time
    inp->i_mtime = time(0L);         // Set last modified to current time
    inp->i_blocks = 0; // # of 512-byte blocks reserved for this inode
	for(i=0;i<15;i++){
		inp->i_block[i] = 0;
	}
	mip->dirty = 1;
	iput(mip);
	
	if(enter_child(pmip, ino, base)!=0){
		printf("enter_child failed, cannot find a space\n");
		return;
	}
}

int my_rmdir(char *pathname){
	int ino, pino, dev, i;
	MINODE *mip, *pmip;
	INODE *ip;
	char sbuf[BLKSIZE], *cp, name[128];
	DIR *dp;
	
	if(!pathname){
		printf("mkdir pathname is NULL\n");
		return -1;
	}
	if(pathname[0]=='/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}
	
	ino = getino(&dev,pathname);
	if(ino==0){
		printf("%s doesn't exist\n",pathname);
		return -1;
	}
	mip = iget(dev,ino);
	if(!S_ISDIR(mip->INODE.i_mode)){
		printf("%s is not a dir\n",pathname);
		iput(mip);
		return -1;
	}
	if(mip->refCount!=1){
		printf("%s refCount = %d\n",pathname,mip->refCount);
		printf("%s is busy\n",pathname);
		iput(mip);
		return -1;
	}
	//check empty
	ip = &(mip->INODE);
	get_block(dev, ip->i_block[0], sbuf);
	cp = sbuf;
	dp = (DIR *)cp;
	cp += dp->rec_len;
	dp = (DIR *)cp;
	cp +=dp->rec_len;
	if((cp - sbuf)!=BLKSIZE){
		printf("%s is not empty\n",pathname);
		return -1;
	}
	findino(mip, &ino, &pino);
	pmip = iget(mip->dev, pino);
	findmyname(pmip, ino, name);
	rm_child(pmip, name);
	
	//truncat(mip);
	for(i = 0; i < 12; i++){
		if(ip->i_block[i] == 0)break;
		bdealloc(dev,ip->i_block[i]);
	}
	idealloc(mip->dev, mip->ino);
	//mip->dirty = 1;
	iput(mip);
	pmip->INODE.i_links_count--;
	pmip->dirty = 1;
	iput(pmip);
	
	return 0;
}

int rm_child(MINODE *pmip, char *name){
	INODE *ip;
	int i, rec_len, found, first_len, second_len;
	char sbuf[BLKSIZE], cpbuf[BLKSIZE], *cp, *precp;
	DIR *dp;
	
	ip = &(pmip->INODE);
	found = 0;
	for(i = 0;i<12;i++){
		if(ip->i_block[i]==0){
			printf("cannot find %s in pmip\n",name);
			return -1;
		}
		get_block(pmip->dev, ip->i_block[i], sbuf);
		cp = sbuf;
		while(cp < sbuf + BLKSIZE){
			dp = (DIR *)cp;
			if(found == 0 && dp->inode!=0 && strcmp(dp->name, name)==0){
				if(dp->rec_len==BLKSIZE){
					dp->inode = 0;
					put_block(pmip->dev, ip->i_block[i], sbuf);
					return 0;
				}
				else if(dp->rec_len + (cp - sbuf) == BLKSIZE){
					dp = (DIR *)precp;
					dp->rec_len = BLKSIZE - (precp - sbuf);
					put_block(pmip->dev, ip->i_block[i], sbuf);
					return 0;
				}
				else{
					found = 1;
					rec_len = dp->rec_len;
					first_len = cp-sbuf;
				}
			}
			if(found == 0)precp = cp;
			cp += dp->rec_len;
		}
		if(found == 1){
			dp->rec_len += rec_len;			
			memcpy(cpbuf, sbuf, first_len);
			memcpy(cpbuf + first_len, sbuf + first_len + dp->rec_len, BLKSIZE-(first_len+rec_len));
			put_block(pmip->dev, ip->i_block[i], cpbuf);
			return 0;
		}
	}
	printf("rm_child not find %s\n",name);
	return -1;
}
#endif