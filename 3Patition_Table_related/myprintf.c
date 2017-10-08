#include <stdio.h>
#include <string.h>
##include "Q1.h"
typedef unsigned int u16;
typedef unsigned int u32;
char *ctable = "0123456789ABCDEF";

//prints
int prints(char *x){
	while(*x){
		putchar(*x);
		x++;
	}
}

//printu
int rpu(u16 x, u16 BASE){
	char c;
	if(x){
		c = ctable[x % BASE];
		rpu(x / BASE, BASE);
		putchar(c);
	}
}

int printu(u16 x){
	(x==0)? putchar('0') : rpu(x,10);
	putchar(' ');
}

//printd
int printd(int x){
	if (x < 0){
		x = -x;
		prints("-");
	}
	(x==0)? putchar('0'):
	printu(x);
}

//printo
int printo(u16 x){
	putchar('0');
	(x==0)? putchar('0') : rpu(x,8);
	putchar(' ');
}

//printx
int printx(u16 x){
	prints("0x");
	(x==0)? putchar('0') : rpu(x, 16);
	putchar(' ');
}

//myprintf
int myprintf(char *fmt, ...){
	char *cp = fmt;
	u16 *ip = (u16 *)&fmt + 1;
	u32 *up; 
	while (*cp){
		if (*cp != '%'){
			putchar(*cp);
			if (*cp =='\n')
				putchar('\r');
			cp++; continue;
		}
		cp++;
		switch(*cp){
			case 'c' : putchar(*ip); break;
			case 's' : prints(*ip); break;
			case 'u' : printu(*ip); break;
			case 'd' : printd(*ip); break;
			case 'o' : printo(*ip); break;
			case 'x' : printx(*ip); break;
		}
		cp++; ip++;
	}
}

int main(int argc, char *argv[ ], char *env[ ]){
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
}


