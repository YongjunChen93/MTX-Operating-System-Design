#ifndef READ_WRITE_H
#define READ_WRITE_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>

#include "util.c"
#include "open_close_lseek.c"

int myread(int fd, char *buf, int nbytes, int debug){
	//(2) count = 0
	// avil = fileSize - OFT's offset // number of bytes still available in file.
	int count_read = 0;
	char *cq = buf;
	int avil = running->fd[fd]->mptr->INODE.i_size - running->fd[fd]->offset;
	int blk =0, lbk = 0, startByte = 0, remain = 0;
	//(3)
	while(nbytes && avil){
       /*(4) Compute LOGICAL BLOCK number lbk and startByte in that block from offset;
        lbk       = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;*/	
        lbk = running->fd[fd]->offset / BLKSIZE;
        startByte = running->fd[fd] -> offset % BLKSIZE;
        //(5) READ
        //(5).1 direct blocks
        if(lbk < 12){
        	blk = running->fd[fd]->mptr->INODE.i_block[lbk];
        }
        //(5).2 Indirect blocks contains 256 block number 
        else if(lbk>=12 && lbk < 256 +12){
        	int indirect_buf[256];
        	get_block(running->fd[fd]->mptr->dev, running->fd[fd]->mptr->INODE.i_block[12],indirect_buf);
        	blk = indirect_buf[lbk-12];
        }
        //(5).3 Double indiirect blocks
        else{
        	int count = lbk -12 -256;
        	/*
			total blocks = count / 256
			offset of certain block = count %256
        	*/
        	int num = count / 256;
        	int pos_offset = count % 256;
        	int double_buf1[256];
        	get_block(running->fd[fd]->mptr->dev, running->fd[fd]->mptr->INODE.i_block[13],double_buf1);
        	int double_buf2[256];
			get_block(running->fd[fd]->mptr->dev,double_buf1[num],double_buf2);
			blk=double_buf2[pos_offset];
        }
        char readbuf[BLKSIZE];
        get_block(running->fd[fd]->mptr->dev, blk, readbuf);
        char *cp = readbuf + startByte;
        remain = BLKSIZE - startByte;
        //(6)
        while(remain){
        	int min = 0;
        	if(avil<=nbytes){
        		min = avil;
        	}
        	else{
        		min = nbytes;
        	}
        	memcpy(cq, cp, min);
        	running->fd[fd]->offset += min;
        	count_read += min;
        	avil -= min;
        	nbytes -= min;
        	remain -= min;
        	if(nbytes == 0 || avil == 0){
        		break;
        	}	
        }
    if(debug ==1){
	    printf("\n*************read file*******************\n");
	    printf("%s", buf);
	    printf("\n*************read file done*******************\n");
	}
    return count_read;
	}	
}

int my_read(int fd, int bytes){
	if((fd>=NFD) || (fd<0)){
		printf("out of fd range\n");
		return -1;
	}
	if(running->fd[fd] == NULL){
		printf("ERROR: must select a valid file descriptor\n");
		return -1;
	}
	if((running->fd[fd]->mode!=0)&&(running->fd[fd]->mode!=2)){
		printf("ERROR: file must be opened for read\n");
		return -1;
	}
	return (myread(fd, read_buff, bytes, 0));

}

int mywrite(int fd, char *buf, int nbytes){
	char *cq = buf;
	char *cp;
	//(2)
	int blk =0, lbk = 0, startByte = 0, remain = 0;
	int nbytes_copy = 0;
	char wbuf[BLKSIZE];
	//(3)
	while(nbytes){
       /*(4) Compute LOGICAL BLOCK number lbk and startByte in that block from offset;
        lbk       = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;*/	
		lbk = running->fd[fd]->offset / BLKSIZE;
		startByte = running->fd[fd]->offset % BLKSIZE;
		//5.1 direct
		if(lbk < 12){
			//if its the first one of the new block, allocatea new one
			if(running->fd[fd]->mptr->INODE.i_block[lbk] == 0){
				running->fd[fd]->mptr->INODE.i_block[lbk] = balloc(running->fd[fd]->mptr->dev);// MUST ALLOCATE a block
			}
			blk = running->fd[fd]->mptr->INODE.i_block[lbk];
		}
		//5.2 indirect 
		else if(lbk >=12 && lbk < 256+12){
			int indirect_buf[256];
			int k = 0;
			//if its the first one of the new block, allocatea new one
			if(running->fd[fd]->mptr->INODE.i_block[12] == 0){
				running->fd[fd]->mptr->INODE.i_block[12] = balloc(running->fd[fd]->mptr->dev);
				memset(indirect_buf,0,BLKSIZE);
				put_block(running->fd[fd]->mptr->dev,running->fd[fd]->mptr->INODE.i_block[12],indirect_buf);
			}
			get_block(running->fd[fd]->mptr->dev,running->fd[fd]->mptr->INODE.i_block[12],indirect_buf);
			if(indirect_buf[lbk-12]==0){
				indirect_buf[lbk-12]=balloc(running->fd[fd]->mptr->dev);
				put_block(running->fd[fd]->mptr->dev,
					running->fd[fd]->mptr->INODE.i_block[12],indirect_buf);
			}
			blk = indirect_buf[lbk-12]; 
		}
		else{
			printf("do not implemnt double indirect blocks yet\n");
			break;
		}
		get_block(running->fd[fd]->mptr->dev, blk, wbuf);
		cp = wbuf + startByte;
		remain = BLKSIZE - startByte;
		while(remain){
			int min = 0;
			if(nbytes<=remain){
				min = nbytes;
			}
			else{
				min = remain;
			}
			memcpy(cp, cq, nbytes);
			nbytes_copy += min;
			nbytes -= min;
			remain -= min;
			running->fd[fd]->offset += min;
			if(running->fd[fd]->offset > running->fd[fd]->mptr->INODE.i_size){
				running->fd[fd]->mptr->INODE.i_size += min;
			}			
			if(nbytes<=0){
				break;
			}
		}
		// write wbuf[ ] to disk
		put_block(running->fd[fd]->mptr->dev, blk, wbuf);
		// loop back to while to write more .... until nbytes are written
	}
	running->fd[fd]->mptr->dirty = 1;       // mark mip dirty for iput() 
	printf("wrote %d char into file descriptor fd=%d\n", nbytes_copy, fd);           
	return nbytes_copy;
}

int my_write(int fd, char *parameter){
	char buf[BLKSIZE];
	int nbytes = 0;
	int i = 0;
	//verify fd is for W
	if(running->fd[fd]->mode == 0){
		printf("fd = %d is not for write\n", fd);
		return -1;
	}
	//copy the text string into buf
	strcpy(buf, parameter);
	nbytes = strlen(buf);
	return (mywrite(fd, buf, nbytes));

}
#endif
