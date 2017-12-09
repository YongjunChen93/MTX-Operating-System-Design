#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>

//LEVEL 1
#include "type.h"
#include "global.c"
#include "util.c"
#include "cd_ls_pwd_quit.c"
#include "mkdir_creat_rmdir.c"
#include "link_unlink.c"
#include "symlink_readlink.c"
#include "chmod_touch.c"

//LEVEL 2
#include "open_close_lseek.c"
#include "read_write.c"
#include "cat_cp_mv.c"

//LEVEL 3
#include "mount_umount.c"

char *disk = "mydisk";

void init(){
    int i, j;
	MINODE *mip;
	PROC *p;
	for (i=0; i<NMINODE; i++){
		mip = &minode[i];
		mip->dev = mip->ino = 0;
		mip->refCount = 0;
		mip->mounted = 0;
		mip->mptr = 0;
	}
	for (i=0; i<NPROC; i++){
		p = &proc[i];
		p->pid = i;
		p->uid = 0;
		p->cwd = 0;
		p->status = FREE;
		for(j=0; j<NFD; j++){
			p->fd[j] = 0;
		}
	}
	printf("initilize done!\n");
}

int mount_root(){

	/*get root inode */
	root = iget(dev, 2);
	rootfilesys = &mounttab[0];
	running = &proc[0];
	running->status = READY;
	running->cwd = iget(dev,2);
	rootfilesys->dev = dev;
	rootfilesys->ninodes = ninodes;
	rootfilesys->nblocks = nblocks;
	rootfilesys->bmap = bmap;
	rootfilesys->imap = imap;
	rootfilesys->iblk = inode_start;
	rootfilesys->mounted_inode = root;
	strcpy(rootfilesys->mount_name, "root");
	strcpy(rootfilesys->name,disk);
	printf("mount root done!\n");
}

void menu(){
	printf("-------          LEVEL 1          ----------\n");
	printf("| mkdir | rmdir |    ls   |     cd    | pwd  | chmod |\n");
	printf("| creat | link  |  unlink |  symlink  | stat | touch |\n");
	printf("-------          LEVEL 2          ----------\n");
	printf("| open[file mode]  | close [fd] | read[fd bytes]  | write[fd strings]|\n");
	printf("| lseek[fd offset] | cat [file] | cp[file1 file2] | mv [file dest]   |\n");
	printf("-------          LEVEL 3          ----------\n");
	printf("| mount [disk loc] | umount [disk] | \n");
	printf("command:");
}

void main(int argc, char *argv[]){
	int ino, ret;
	if (argc>1) disk = argv[1];
	printf("checking EXT2 FS ....\n");
	if ((fd = open(disk, O_RDWR)) < 0){
		printf("open %s failed\n", disk);
		exit(1);
	}
	dev = fd;
	char buf[BLKSIZE];
	//read super block at 1024
	get_block(dev,1,buf);	
	sp = (SUPER *)buf;
	//verify it's an ext2 file system
	if (sp->s_magic !=0xEF53){
		printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
		exit(1);
	}
	/*mtable[0] to record, get inform from superblock and GD*/
	ninodes = sp->s_inodes_count;
	nblocks = sp->s_blocks_count;
	get_block(dev,2,buf);
	gp = (GD *)buf;
	bmap = gp->bg_block_bitmap;
	imap = gp->bg_inode_bitmap;
	inode_start = gp->bg_inode_table;
	init();
	mount_root();
	/* exe command*/
	int (*fptr[])(char*) = {(int (*)())my_ls, my_cd, my_pwd, my_mkdir, my_creat, my_rmdir};
	int (*f2ptr[])(char*, char*)={(int (*)())my_link, my_unlink, my_symlink, my_readlink,
										 my_chmod, my_touch, quit,my_open};

	while(1){
		menu();
		fgets(line, 128, stdin);
		line[strlen(line)-1] = 0;
		if(line[0]==0) continue;
		pathname[0] = 0;
		parameter[0] = 0;
		sscanf(line, "%s%s%s", cmd, pathname, parameter);
		printf("cmd = %s, pathname = %s, parameter = %s\n",cmd,pathname,parameter);
		int index = findCmd(cmd);
		if(index == -1){
			printf("invalid command\n");
		}
		else if(index >= 0 && index <= 5){
			fptr[index](pathname);
		}
		else if(index > 5 && index <= 8 || index >9 && index<=11){
			f2ptr[index-6](pathname,parameter);
		}
		else if(index == 9){
			f2ptr[index-6](pathname,buf);
		}
		else if(index == 12){
			quit();
		}
		else if(index == 13){
			f2ptr[index-6](pathname,atoi(parameter));
		}
		else if(index == 14){
			my_close(atoi(pathname));
		}
		else if(index == 15){
			mylseek(atoi(pathname),atoi(parameter));
		}
		else if(index == 16){
			my_read(atoi(pathname), atoi(parameter));
		}
		else if(index == 17){
			my_write(atoi(pathname),parameter);
		}
		else if(index == 18){
			my_cat(pathname);
		}
		else if(index == 19){
			my_cp(pathname, parameter);
		}
		else if(index == 20){
			my_mv(pathname, parameter);
		}
		else if(index == 21){
			my_mount(pathname, parameter);
		}
		else if(index == 22){
			my_umount(pathname);
		}
		else{
			printf("Invalid command");
			return;
		}
	}
	return 0;
}