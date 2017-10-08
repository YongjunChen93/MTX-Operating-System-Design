
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "myprintf.c"

typedef unsigned char u8;
//typedef unsigned short u16;
typedef unsigned int u32;
int fd;

struct partition{       
    u8 drive;             /* 0x80 - active */
	u8  head;             /* starting head */
	u8  sector;           /* starting sector */
	u8  cylinder;         /* starting cylinder */
	u8  sys_type;         /* partition type */
	u8  end_head;         /* end head */
	u8  end_sector;       /* end sector */
	u8  end_cylinder;     /* end cylinder */
	u32 start_sector;     /* starting sector counting from 0 */
	u32 nr_sectors;       /* number of of sectors in partition */
};

int main(int argc, char *argv[ ], char *env[ ]){  
	myprintf("---------Solution of question 1 & 2---------\n");
	myprintf("===argc, argv, env===\n");
	myprintf("argc = %d\n",argc);
	for(int i = 0; i < argc; i++){
		myprintf("argv[%d] = %s\n", i, argv[i]);
	}
	int j = 0;
	while(env[j]){
		myprintf("env[%d] = %s\n", j, env[j]);
		j++;
	}
	myprintf("===test myprintf===\n");
	myprintf("cha=%c string=%s dec=%d hex=%x oct=%o neg=%d\n",
		'A',"this is a test", 100,100,100,-100);
	myprintf("=====end test=====\n");
	
	myprintf("---------Solution of question 3-----------\n");
	char buf[512];
	int sector, startsect,r;
	struct partition *p;
	fd = open("vdisk", O_RDONLY);          // open disk iamge file for READ
	r = read(fd, buf, 512);                // read FIRST 512 bytes into buf[ ]
	p = (struct partition *)(&buf[0x1BE]); // p points at Ptable in buf[ ]
	myprintf("----- RAW form ------\n");	
	for(int i=0; i<4; i++){
		myprintf(" %d ",p->drive);
		myprintf("%d ",p->head);
		myprintf("%d ",p->sector);
		myprintf("%d ",p->cylinder);
		myprintf("%d ",p->sys_type);
		myprintf("%d ",p->end_head);
		myprintf("%d ", p->end_sector);
		myprintf("%d ",p->end_cylinder);
		myprintf("%d ",p->start_sector);
		myprintf("%d\n",p->nr_sectors);
		p++;
		if(p->sys_type == 5){
			sector = p->start_sector;
		}
	}
	myprintf("----- Linux fdisk form -----\n");
	myprintf("start end size\n");
	p = (struct partition *)(&buf[0x1BE]); // p points at Ptable in buf[ ]
	for (int i=0;i<4;i++){
		myprintf("%d ",p->start_sector);
		myprintf("%d ",p->nr_sectors+p->start_sector-1);
		myprintf("%d\n",p->nr_sectors);
		p++;
	}
	// Assume you want to read sector 10 (counting from 0)
	myprintf("***** Look for Extend Partition *****\n");
	myprintf("Ext Partition _ start sector =  %d\n",sector);
	myprintf("start  end  size\n");
	while(1){
		myprintf("load sector ->%d\n",sector);
		lseek(fd, (long)(sector*512), 0);     // seek to sector 10           
		read(fd, buf, 512);                   // read sector 10 into buf[ ], etc.
		p = (struct partition *)(&buf[0x1BE]); 
		myprintf("LOCAL MBR start_sector = %d ext partition start_sector = %d\n",sector,p->start_sector);
		myprintf("%d ", sector+p->start_sector);
		myprintf("%d ", sector+p->start_sector+p->nr_sectors-1);
		myprintf("%d\n", p->nr_sectors);
		p++;
		sector = sector + p->nr_sectors;
		if((p->nr_sectors)==0){
			break;
		}
	}
}
