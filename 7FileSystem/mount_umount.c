#ifndef MOUNT_UMOUNT_H
#define MOUNT_UMOUNT_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

#include "util.c"

int my_mount(char *filesys, char *mount_point){	
	int i,new_dv,dir_ino,dev;
	MOUNT *pmount;
	MINODE *pdir_mip;
	char buf[BLKSIZE];
	SUPER *sp;
    GD *gp;	
    //(1)
	if(strlen(filesys) == 0 && strlen(mount_point) == 0){
		display_mount(rootfilesys);
		return 0;
	}	
	if(strlen(filesys) == 0 || strlen(mount_point) == 0){
		printf("In mount, filesys = %s, mount_point = %s are not complete\n",filesys,mount_point);
		return -1;
	}
	//(2)
	for(i=0;i<NMOUNT;i++){
		if(strcmp(mounttab[i].name,filesys)==0){
			printf("In mount, %s already mounted\n",filesys);
			return -1;
		}
	}
	for(i=0;i<NMOUNT;i++){
		if(mounttab[i].dev==0){
			pmount = &mounttab[i];
			break;
		}
	}
	new_dv = open(filesys, O_RDWR);
	if(new_dv <= 0){
		printf("In mount: %s could not be opened\n",filesys);
		return -1;
	}
	//read super block at 1024
	get_block(new_dv,1,buf);	
	sp = (SUPER *)buf;
	//verify it's an ext2 file system
	if (sp->s_magic !=0xEF53){
		printf("In mount, magic = %x is not an ext2 filesystem\n", sp->s_magic);
		return -1;
	}
	if(mount_point[0]=='/'){
		dev = root->dev;
	}
	else{
		dev = running->cwd->dev;
	}
	//(4)
	dir_ino = getino(&dev,mount_point);
	if(dir_ino==0){
		printf("In mount, mount_point = %s doesn't exist\n",mount_point);
		return -1;
	}
	pdir_mip = iget(dev,dir_ino);
	//(5)check is a DIR
	if(!S_ISDIR(pdir_mip->INODE.i_mode)){
		printf("In mount, mount_point = %s is not a dir\n",mount_point);
		iput(pdir_mip);
		return -1;
	}
	//(5) check is not busy
	for(i=0;i<NPROC;i++){
		if(proc[i].cwd == pdir_mip){
			printf("In mount, mount_point = %s is busy \n",mount_point);
		    iput(pdir_mip);
			return -1;
		}
	}	
	//(6) - (7)
	pdir_mip->mptr = pmount;
	pmount->dev = new_dv;
	pmount->ninodes = sp->s_inodes_count;
	pmount->nblocks = sp->s_blocks_count;
	get_block(dev,2,buf);
	gp = (GD *)buf;
	pmount->bmap = gp->bg_block_bitmap;
	pmount->imap = gp->bg_inode_bitmap;
	pmount->iblk = gp->bg_inode_table;
	pmount->mounted_inode = pdir_mip;
	strcpy(pmount->mount_name, mount_point);
	strcpy(pmount->name,filesys);
	pdir_mip->mounted = 1;
	printf("mount %s succeed\n", filesys);
    return 0;	
}


int my_umount(char *filesys){
    int i;
	MOUNT *pmount = NULL;
	MINODE *pdir_mip;
	//(1)
	for(i=0;i<NMOUNT;i++){
		if(strcmp(mounttab[i].name,filesys)==0){
			pmount = &mounttab[i];
			break;
		}
	}
	if(pmount == NULL){
		printf("In umount, %s is not mounted\n",filesys);
		return -1;
	}
	//(2) check any file in dev is active or not
	for(i=0;i<NMINODE;i++){
		if(minode[i].refCount!=0){
			if(minode[i].dev == pmount->dev){
				printf("In umount, filesys %s is busy\n",filesys);
				return -1;
			}
		}
	}
	//(3)
	display_mount(pmount);
	pmount->name[0] = 0;
	pmount->dev = 0;
	pdir_mip = pmount->mounted_inode;
	pdir_mip->mounted = 0;
	iput(pdir_mip);	
	printf("umount %s filesys succeed!\n", filesys);
	//(4)
	return 0;
}

display_mount(MOUNT *pmount){
	printf("dev = %d\t nblocks = %d\t ninodes = %d\t bmap = %d\t imap = %d\t iblk = %d\nname = %s\t mount_name = %s\n",
	    pmount->dev,pmount->nblocks,pmount->ninodes,pmount->bmap,pmount->imap,pmount->iblk,pmount->name,pmount->mount_name);
}

#endif