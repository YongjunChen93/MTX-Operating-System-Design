#ifndef UTIL_C_H
#define UTIL_C_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <sys/stat.h>

#include "type.h"
#include "global.c"

//find command
int findCmd(char *command){
	int i = 0;
	while(CMD[i]){
		if(strcmp(command,CMD[i])==0)
			return i;
		i++;
	}
	return -1;
}

int get_block(int fd, int blk, char buf[])
{
    lseek(fd, (long)blk*BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[])
{
    lseek(fd, (long)blk*BLKSIZE, 0);
    write(fd, buf, BLKSIZE);
}

int indexof(char* s,char delim)
{
    int ret = -1;
    int i = 0;
    while(s[i])
    {
        if(s[i]==delim)
        {
            ret = i;
            break;
        }
        i++;
    }
    return ret;
}

int lastindexof(char* s,char delim)
{
    int ret = -1;
    int i = 0;
    while(s[i])
    {
        if(s[i]==delim) ret = i;
        i++;
    }
    return ret;
}

void tokenize(char *pathname)
{
	n = 0;
	char *p = pathname;	
	if(NULL==p)return;
	int index;
	while(*p)
	{
		index = indexof(p,'/');
		if(-1==index)
		{
			strcpy(names[n],p);
			name[n] = names[n];
			n++;
			return;
		}
		if(0==index)
		{
			p++;
			continue;
		}
		strncpy(names[n],p,index);
		names[n][index] = '\0';
		name[n] = names[n];
		p += index+1;
		n++;
	}
}

MINODE* iget(int dev,int ino)
{
	int i;
	for(i=0;i<NMINODE;i++)
	{
		if(minode[i].refCount>0)
		{
			if(dev==minode[i].dev && ino==minode[i].ino)
			{
				minode[i].refCount++;
				return &minode[i];
			}
		}
	}
	char buf[BLKSIZE];
	for(i=0;i<NMINODE;i++)
	{
		if(minode[i].refCount==0)
		{
			MINODE *mip = &minode[i];
			//Mailman's algorithm
			int blk = (ino-1)/8 + inode_start;
			int offset = (ino-1)%8;
			
			get_block(dev,blk,buf);
			INODE *ip = (INODE *)buf+offset;
			mip->INODE = *ip;
			mip->dev = dev;
			mip->ino = ino;
			mip->refCount = 1;
			mip->dirty = 0;
			mip->mounted = 0;
			mip->mptr = 0;
            return mip;			
		}
	}
	return NULL;
}

int getino(int *dev,char *pathname)
{
	tokenize(pathname);
	MINODE *mip;
	int i, ino;
	if(pathname[0]=='/') 
	{
		*dev = root->dev;
		mip = root;
		ino = 2;//default root
	}
    else
	{
		*dev = running->cwd->dev;
		mip = running->cwd;
		ino = mip->ino;
	}	
	for(i=0;i<n;i++)
	{
		if(strcmp("..",name[i])==0)
		{
			if(2==ino && *dev != root->dev)
			{
				for(i=0;i<NMOUNT;i++)
				{
					if(mounttab[i].dev == *dev)
					{
						iput(mip);
						mip = mounttab[i].mounted_inode;
						*dev = mip->dev;
						ino = search(mip,name[i]);
						break;
					}
				}
				continue;
			}	
		}		
		ino = search(mip,name[i]);
		
		if(0==ino) return 0;
        				
		mip = iget(*dev,ino);
		
		if(mip->mounted = 1)
		{
			iput(mip);
			if(mip->mptr == NULL) continue;
			*dev = mip->mptr->dev;
			ino = 2;
			mip = iget(*dev,ino);//root
		}
		else
		{			
		    iput(mip);
		}
	}
	printf("get ino of %s at (%d,%d)\n",pathname,*dev,ino);
	return ino;
}

int search(MINODE *mip, char *name)
{
	int i; 
    char *cp, sbuf[BLKSIZE];
    DIR *dp;
    INODE *ip;

    ip = &(mip->INODE);
    for (i=0; i<12; i++){  // ASSUME DIRs only has 12 direct blocks
        if (ip->i_block[i] == 0)
          return 0;

        //get ip->i_block[i] into sbuf[ ];
      	//READ contents of directory entires
	    get_block(mip->dev, ip->i_block[i], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;
    	printf(" i_number rec_len name_len   name\n");
        while (cp < sbuf + BLKSIZE){
    		char temp=dp->name[dp->name_len];
    		dp->name[dp->name_len]=0;
    		printf("%8d %8d %4d %10s\n",dp->inode, dp->rec_len,dp->name_len,dp->name);        	
            if(strcmp(name,dp->name)==0)
			{
				return dp->inode;
			}
    		dp->name[dp->name_len]=temp;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
    return 0;
}

int iput(MINODE *mip)
{
	mip->refCount--;
	if(mip->refCount > 0) return 0;
	if(mip->dirty == 0) return 0;
	
	//Mailman's algorithm
	int blk = (mip->ino-1)/8 + inode_start;
    int offset = (mip->ino-1)%8;
	
	char buf[BLKSIZE];
	get_block(mip->dev,blk,buf);
	INODE *ip = (INODE *)buf+offset;
	*ip = mip->INODE;
	//printf("in iput, i_block[0] = %d\n",ip->i_block[0]);
	put_block(mip->dev, blk, buf);	
}

int findmyname(MINODE *parent, int myino, char *myname) 
{
   	int i;
    char *cp, sbuf[BLKSIZE];
    DIR *dp;
    INODE *ip;

    ip = &(parent->INODE);
    for (i=0; i<12; i++){  // ASSUME DIRs only has 12 direct blocks
        if (ip->i_block[i] == 0)
          return 0;

        //get ip->i_block[i] into sbuf[ ];
	    get_block(parent->dev, ip->i_block[i], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;
        while (cp < sbuf + BLKSIZE){
            if(dp->inode == myino)
			{
				strcpy(myname,dp->name);
				return myino;
			}
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
    return 0;
}

int findino(MINODE *mip, int *myino, int *pino)
{
	if(NULL==mip) 
	{
		printf("in func findino, mip is NULL\n");
		return -1;
	}
	
	if(!S_ISDIR(mip->INODE.i_mode))
	{
		printf("this is not a dir\n");
		return -1;
	}
	
    char *cp, sbuf[BLKSIZE];
    DIR *dp;
    INODE *ip;
	
	ip = &(mip->INODE);
	if (ip->i_block[0] == 0) return 0;
	get_block(mip->dev, ip->i_block[0], sbuf);
	dp = (DIR *)sbuf;
    cp = sbuf;
	*myino = dp->inode;
	cp += dp->rec_len;
    dp = (DIR *)cp;
	*pino = dp->inode;
	return mip->ino;
}

#endif