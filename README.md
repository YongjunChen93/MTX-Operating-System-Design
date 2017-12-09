# Design and Implement MTX Operating System
# - Contents

```
1. Basic Understanding of Operating System
2. Stack Usage, Design Myprintf function
3. Partition table, fdisk, structures in C, read special files
4. Simulate Unix File System Tree
5. Sh Simulator
6. File Operation Cross Networks
7. File System design
```

## 7.File System
```
Designed a file system includes following commands:
              -------  LEVEL 1 ------------ 
               mount_root;
               mkdir, rmdir, ls, cd, pwd;
               creat, link,  unlink, symlink
               stat,  chmod, touch;
              -------  LEVEl 2 -------------
               open,  close,  read,  write
               lseek  cat,    cp,    mv
              -------  LEVEl 3 ------------ 
               mount, umount
              -----------------------------
All the commands works exactly the same way as they do in Unix/Linux.
```

## Acknowledge
The works are done based on the Linux system and C programming language. The whole works design and simulate a complete advanced operating systems. Here I want specifically thanks for professor KC Wang's teaching.


```
## Reference

The code was implemented based on the book Design and Implementation of the MTX Operating System, K.C. Wang, Springer International AG, 2015. 

## Acknowledge
This course is taken in 2017 Fall, it's really an interesting and useful class. It not only helpes me understand the operation systems clearly, but also helps me improve my C programming language skills dramatically .
