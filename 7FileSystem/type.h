#ifndef TYPE_H_H
#define TYPE_H_H

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef struct ext2_super_block SUPER;
typedef struct ext2_group_desc  GD;
typedef struct ext2_inode       INODE;
typedef struct ext2_dir_entry_2 DIR;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR   *dp;   

// Block number of EXT2 FS on FD
#define SUPERBLOCK        1
#define GDBLOCK           2
#define ROOT_INODE        2

// Default dir and regulsr file modes
#define DIR_MODE    0040777 
#define FILE_MODE   0100644
#define SUPER_MAGIC  0xEF53
#define SUPER_USER        0

// Proc status
#define FREE              0
#define READY             1
#define RUNNING           2

// File mode
#define RDONLY        0
#define WRONLY        1
#define RDWR          2
#define APPEND        3
#define CREAT         4

#define BLKSIZE     1024
#define BLOCK_SIZE  1024

// Table sizes
#define NMINODE         100
#define NMOUNT           10
#define NPROC            10
#define NFD              10
#define NOFT            100

// In-memory inodes structure
typedef struct minode{
  INODE INODE;
  int dev, ino;
  int refCount;
  int dirty;
  int mounted;
  struct mount *mptr;
}MINODE;

// Open File Table
typedef struct oft{
  int  mode;
  int  refCount;
  MINODE *mptr;
  int  offset;
}OFT;

// PROC structure
typedef struct proc{
  struct proc *next;
  int          pid;
  int          ppid;
  int          status;
  int          uid, gid;
  MINODE      *cwd;
  OFT         *fd[NFD];
}PROC;

// Mount Table structure
typedef struct mount{
  int    dev;
  int    nblocks,ninodes;
  int    bmap, imap, iblk;
  MINODE *mounted_inode;
  char   name[64]; 
  char   mount_name[64];
}MOUNT;

#endif