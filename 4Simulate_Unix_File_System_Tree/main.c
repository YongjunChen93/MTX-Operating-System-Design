#include "actionfunctions.c"

int main(int argc, char *argv[]){	
	initialize();//initialize / node of the file system tree
	printf("Root initialized OK\n");
	printf("Enter ? for help menu\n");
	cwd=root;//current directory is root
	int (*fptr[])(char*)={(int (*)())mkdir, rmdir, cd, ls, pwd, creat, rm};
	while(1){
		printf("Command : ");
		fgets(line,128,stdin);//get the user input line   
		sscanf(line,"%s %s", command, pathname);
		int index=findCmd(command);
		if(index == -1){
			printf("invalid command\n");
		}
		else if(index >= 0 && index <= 6){
			fptr[index](pathname);
		}
		else{
			switch(index){
				case 7: save(); break;
				case 8: reload(); break;
				case 9: menu(); break;
				case 10: quit(); break;	
			}
		}
	}
	return 0;
}


