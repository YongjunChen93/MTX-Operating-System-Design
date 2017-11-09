#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "util.c"

printdir(MINODE *mip)
{
	//printf("enter print(MINODE *mip)\n");
	int i; 
    char *cp, sbuf[BLKSIZE];
    DIR *dp;
    INODE *ip;
	MINODE *cmip;

    ip = &(mip->INODE);
	//printf("after ip = &(mip->INODE)\n");
	for (i=0; i<12; i++){  // ASSUME DIRs only has 12 direct blocks
	    //printf("ip->i_block[%d] = %d\n",i,ip->i_block[i]);
        if (ip->i_block[i] == 0)
          return 0;

        //get ip->i_block[i] into sbuf[ ];
	    get_block(mip->dev, ip->i_block[i], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;
        while (cp < sbuf + BLKSIZE){
			cmip = iget(mip->dev,dp->inode);
			printfile(&(cmip->INODE),dp->name);
			iput(cmip);
            //printf("%s\n",dp->name);
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
}

printfile(INODE *ip,char *name)
{
	//printf("enter printfile\n");
	int i;
	char line_buf[256], item_buf[256];
	memset(line_buf, 0, 256);
    memset(item_buf, 0, 256);
	
	if(!ip)
	{
		printf("INODE address is NULL\n");
		return;
	}
	u16 mode   = ip->i_mode;
    u16 links  = ip->i_links_count;
    u16 uid    = ip->i_uid;
    u16 gid    = ip->i_gid;
    u32 size   = ip->i_size;
	
	static const char* Permissions = "rwxrwxrwx";
	
	//printf("before switch\n");
	switch(mode & 0xF000) 
    {
        case 0x8000:  putchar('-');     break; // 0x8 = 1000
        case 0x4000:  putchar('d');     break; // 0x4 = 0100
        case 0xA000:  putchar('l');     break; // oxA = 1010
        default:      putchar('?');     break;
    }
	
	//printf("after switch\n");
	// Permissions
    for(i = 0; i < strlen(Permissions); i++)
        putchar(mode & (1 << (strlen(Permissions) - 1 - i)) ? Permissions[i] : '-');
	
	char *time = ctime((const time_t*)(&ip->i_mtime));
	time[strlen(time)-1] = 0;
			
	printf("%4hu %4hu %4hu %8u %s %s", 
        links, gid, uid, size, time, name);
	
	//printf("before Trace link\n");
	// Trace link
    if(S_ISLNK(ip->i_mode))
        printf(" -> %s", (char*)ip->i_block);
	putchar('\n');
}

int my_ls(char *pathname)
{
	int ino, dev = running->cwd->dev;
	MINODE *mip = running->cwd;
	if (strlen(pathname)>0)
	{
		if (pathname[0]=='/')
			dev = root->dev;
		ino = getino(&dev, pathname);
		if(ino == 0)
		{
			printf("%s doesn't exist\n",pathname);
			return -1;
		}
		mip = iget(dev, ino);
	}
	if(S_ISDIR(mip->INODE.i_mode))
	{
		printdir(mip);
	}
	else
	{
		printfile(&(mip->INODE),basename(pathname));
	}
		
	if(strlen(pathname)>0)
	{
		iput(mip);
	}
	return 0;
}

int my_cd(char *pathname)
{
	MINODE *mip;
	int dev;
	
    if (strlen(pathname)==0 || (strlen(pathname)==1 && pathname[0]=='/'))
	{		
        mip = root;
		iput(running->cwd);
	    running->cwd = mip;
		running->cwd->refCount++;
	}
    else
	{
		if(pathname[0]=='/')
		{
			dev = root->dev;
		}
		else
		{
			dev = running->cwd->dev;
		}
		int ino = getino(&dev,pathname);
		if(ino <=0)
		{
			printf("%s doesn't exist\n",pathname);
			return -1;
		}
		printf("ino = %d\n",ino);		
		mip = iget(dev,ino);
		if(!S_ISDIR(mip->INODE.i_mode))
		{
			iput(mip);
			printf("%s is not a dir\n",pathname);
			return -1;
		}
		iput(running->cwd);
	    running->cwd = mip;
		running->cwd->refCount++;
	}	
	return 0;
}

my_pwd()
{
	printf("enter pwd\n");
	if(running->cwd==root)
	{
		printf("/\n");
		return;
	}
	printcwd(running->cwd);
	
	printf("\n");
}

printcwd(MINODE *mip)
{
	int myino, parentino, i, dev;
	//printf("enter printcwd\n");
	char myname[128];
	MINODE *parent;
	
	dev = mip->dev;
	
	//printf("before findino\n");
	findino(mip,&myino,&parentino);
	//printf("after findino\n");
	//printf("myino=%d,parentino=%d\n",myino, parentino);
	if(myino==2)
	{
		if(mip->dev == root->dev)
		{
			//printf("/");
			return;
		}
		for(i=0;i<NMOUNT;i++)
		{
			if(mounttab[i].dev == mip->dev)
			{
				mip = mounttab[i].mounted_inode;
				parentino = mip->ino;
				dev = mip->dev;
				break;
			}
		}		
	}
	
	if(parentino!=0)
	{
		parent = iget(dev,parentino);
	    //printf("after iget\n");
	    findmyname(parent, myino, myname);
	    //printf("after findmyname\n");
	    printcwd(parent);
		
		iput(parent);
	    if(myino != 2) 
		{
			printf("/");
			printf("%s",myname);			
		}	
	}

}
int quit(){
	int i;
	MINODE *mip;
	for(i=0; i<NMINODE; i++){
		mip = &minode[i];
		if(mip->refCount>0)
			iput(mip);
	}
	display_mount(rootfilesys);
	exit(0);
}