#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>//to get home directory, chdir()
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <libgen.h>
#include <assert.h>
#include <sys/wait.h>


void decompose(char *input, char* token, char** array);
void childExe(char* argv[],char *envp[]);
void UsePipe(char **argv,char **head, char **tail, char *envp[]);
void forkchild(char *envp[]);
void Johnsh(char *envp[]);

//global variables
char line[256];
char oriline[256];
char *myargv[100];
char *dirarray[100];

void decompose(char *input, char* token, char** array){
	int index=0;
	char *p=strtok(input,token);
	while(p!=NULL){
		array[index++]=p;
		p=strtok(NULL,token);
	}
	array[index]=NULL;
}

void childExe(char* argv[],char *envp[]){
	int i=0;
	while(argv[i]!=NULL){
		printf("%d %s\n",i,argv[i]);
		if(strcmp(argv[i],"<")==0){
			int fdin=open(argv[i+1],O_RDONLY);
			dup2(fdin,0);//0 will point to fdin-read from the file instead of the keyboard
			close(fdin);//close a file descriptor
			break;
		}
		else if(strcmp(argv[i],">")==0){
			int fdout=open(argv[i+1],O_WRONLY | O_CREAT,0600);//600 mode-permission: the owner can read and write
			dup2(fdout,1);//write to the file instead of the screen
			close(fdout);
			break;
		}
		else if(strcmp(argv[i],">>")==0){
			int fdout=open(argv[i+1],O_WRONLY | O_APPEND);
			dup2(fdout,1);
			close(fdout);
			break;
		}
		i++;
	}
	argv[i]=NULL;
	char temp[100];
	strcpy(temp,argv[0]);
	argv[0] = NULL;
	int dirindex=0;
	int count = 0;
	int n = 0;
	while(dirarray[dirindex]!=NULL) {		
		char newcmd[100];
		strcpy(newcmd,"");
		strcpy(newcmd,dirarray[dirindex]);
		strcat(newcmd,"/");
		argv[0] = newcmd;
		strcat(argv[0],temp);
		printf("name[0]=%s i = %d cmd=%s\n",temp,count++,argv[0]);			
		execve(argv[0],argv,envp);
		dirindex++;
	}	
}

void UsePipe(char **argv,char **head, char **tail, char *envp[]){
	//no pipe
	if(head==tail && head==NULL){
		printf("NO pipe: head = %s\n",oriline);
		childExe(myargv,envp);
	}
	//has pipe
	else{
		printf("Has pipe head cmd = %s, tail cmd = %s\n", head[0],tail[0]);
		int pd[2];
		//printf("pd---->%d\n",pd[1]);
		pipe(pd);
		int pid=fork();
		if(pid<0){
			perror("fork failed\n");
			exit(1);
		}
		if(pid){		   // parent as writer 
			printf("parent   ----\n");
			close(pd[0]);
			dup2(pd[1], 0);
			childExe(head,envp);			
		}
		else{
			close(pd[1]); // child as reader 
			dup2(pd[0], 0); 
			childExe(tail,envp);
		}
	}
}

void forkchild(char *envp[]){
	int status;
	int pid;
	pid=fork();//fork syscall;parent returns child pid, child returns 0
	if(pid<0)//fork() fails
	{
		perror("fork failed\n");
		exit(1);
	}
	if(pid) //parent executes this step
	{
		printf("parent JohnChensh PROC %d forks a child process %d\n",getppid(),getpid());
		printf("parent sh PROC %d waits\n",getppid());
		pid=wait(&status);//wait for the child to terminate
		printf("child sh PROC %d died : exit status = %04x\n",getpid(),status);
	}
	else //child executes this step
	{
		printf("PROC %d do command: line=%s\n",getpid(),oriline);
		int index=0;
		char **head=NULL;
		char **tail=NULL;
		while(myargv[index]!=NULL)
		{
			if(strcmp(myargv[index],"|")==0)
			{
				myargv[index]=NULL;
				head=myargv;
				tail=myargv+index+1;
				break;
			}
			index++;
		}
		UsePipe(myargv,head, tail, envp);
	}
}

void Johnsh(char *envp[]){
	//cmd = cd 
	if(strcmp(myargv[0],"cd")==0){
		//chdir($HOME)
		if(myargv[1]==NULL){
			chdir(getenv("HOME"));
		}
		//chdir(arg1)
		else{
			chdir(myargv[1]);
		}
	}
	//cmd = exit
	else if(strcmp(myargv[0], "exit")==0){
		exit(1);
	}
	else{
		forkchild(envp);
	}
}

int main(int argc, char *argv[], char *envp[]){
	printf("************* Welcome to John Chen sh **************\n");
	printf("1. show PATH:\n");
	char *pathname;
	if(pathname = getenv("PATH")){
		printf("PATH=%s\n",pathname);
	}
	printf("2. decompose PATH into dir strings:\n");
	char *curpath;
	int index = 0;
	curpath = strtok(pathname,":");
	dirarray[index]=curpath;
	printf("%s  ",curpath);
	while(curpath = strtok(0,":")){
		printf("%s  ", curpath);
		dirarray[index++] = curpath;
	}
	printf("\n3. show HOME directory: ");
	printf("HOME = %s\n",getenv("HOME"));
	printf("4: *********** John Chen processing loop **********\n");
	while(1){
		printf("John Chen$: ");
		fgets(line,256,stdin);
		if(strlen(line)!=1){
			line[strlen(line)-1]='\0';
			strcpy(oriline,line);
			decompose(line," ",myargv);	
			Johnsh(envp);
		}
	} 
	return 0;
}
