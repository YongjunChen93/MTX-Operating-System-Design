#include "actionfunctions.h"
#include "auxiliaryfunctions.c"

/*================= find command: return an index ========================*/
char *cmd[] = {"mkdir", "rmdir","cd","ls","pwd","creat","rm","save","reload", "?", "quit", 0};

/*------------------- Action Functions -------------------*/
// initialize command
//initialize / node of the file system tree
void initialize(){
	//allocate memory for the root
	root=(NODE *)malloc(sizeof(NODE));
	root->node_type='D';
	strcpy(root->name,"/");
	root->childPtr=NULL; 
	//*siblingPtr, *parentPtr point to themselves
	root->siblingPtr=root;
	root->parentPtr=root;

	cwd = (NODE *)malloc(sizeof(NODE));
	cwd->node_type='D';
	strcpy(root->name,"/");
	cwd->childPtr=NULL; 
	cwd->siblingPtr = cwd;
	cwd->parentPtr=cwd;
}

//find command
int findCmd(char *command){
   int i = 0;
   while(cmd[i]){
     if (strcmp(command, cmd[i])==0)
         return i;
     i++;
   }
   return -1;
}

//mkdir command
//pathname[64];//user inputs
//char dirname[64], basename[64];//string holders

void mkdir(char *pathname){
	if(strcmp(pathname, "/")==0){
		printf("sorry, can't mkdir /\n");
		return;
	}
	char temp[128]={'/'};
	//absolute or relative 
	(pathname[0]=='/')?strcpy(temp,pathname):strcat(temp,pathname);	
	strcpy(DirName,dirname(temp));
	printf("dirname = %s ",DirName);
	strcpy(BaseName,basename(temp));
	printf("basename = %s\n",BaseName);	
	printPath(DirName);			
	NODE *p1;			
	if(pathname[0]=='/'){
		p1=iterativeLevelSearch(root, DirName, 'D');
	}
	else{
		p1=iterativeLevelSearch(cwd, DirName, 'D');
	}
	if(p1!=NULL){
		NODE *x=levelSearch(p1,BaseName, 'D');
		if(x==NULL){
			insert(p1, BaseName,'D');
			printf("---- mkdir OK ----\n");
		}
		else{
			printf("\n---- mkdir : %s already exits\n",BaseName);
		}
	}		
}

//rmdir command
void rmdir(char *pathname){
	if(strcmp(pathname, "/")==0){
		printf("pathname=/ : n = 0  ==>\n");
		printf("sorry, can't mkdir /\n");
	}
	//absolute or relative location
	char temp[128]={'/'};
	(pathname[0]=='/')?strcpy(temp,pathname):strcat(temp,pathname);
	strcpy(DirName,dirname(temp));
	printf("dirname = %s  ",DirName);
	strcpy(BaseName,basename(temp));
	printf("basename = %s\n",BaseName);	
	printPath(DirName);			
	NODE *p1=iterativeLevelSearch(root, DirName, 'D');
	if(p1!=NULL){
		NODE *x=levelSearch(p1,BaseName,'D');
		if(x==NULL){
			printf("can't find %s\n",BaseName);
			printf("\n---- rmdir : no such dir ----\n");
		}
		else if(x->node_type!='D'){
			printf("%s : non-DIR\n",BaseName);
		}
		else if(checkEmpty(x)==1){
			printf("---- rmdir : dir not empty ----\n");
		}
		else{
			delete(x);
			printf("\n---- rmdir OK ----\n");
		}
	}		
}

//cd command
void cd(char *pathname){
	//.. 
	if(strcmp(pathname,"..")==0){
		cwd=cwd->parentPtr;
		printf("cwd:%s\n",cwd->name);
		return;
	}
	char *token;
	char temp[128]={'/'};
	(pathname[0]=='/')?strcpy(temp,pathname):strcat(temp,pathname);	
	strcpy(DirName,dirname(temp));
	strcpy(BaseName,basename(temp));
	NODE *p1=iterativeLevelSearch(cwd, DirName, 'D');
	if(p1!= NULL){
		NODE *x=levelSearch(p1,BaseName, 'D');
		if(x!=NULL){
			cwd = x;
			printf("cwd:%s\n",cwd->name);
		}
	}
	else{
		printf("------incorrect location------\n");
	}

}

//ls command
void ls(char *pathname){
	if(strcmp(pathname, "")==0){
		traverseChild(cwd);
		return;
	}
	if(strcmp(pathname,"/")==0){
		traverseChild(root);
		return;
	}
	char *token;
	char temp[128]={'/'};
	(pathname[0]=='/')?strcpy(temp,pathname):strcat(temp,pathname);	
	strcpy(DirName,dirname(temp));
	strcpy(BaseName,basename(temp));
	NODE *p1;			
	if(pathname[0]=='/'){
		p1=iterativeLevelSearch(root, DirName, 'D');
	}
	else{
		p1=iterativeLevelSearch(cwd, DirName, 'D');
	}
	if(p1!= NULL){
		NODE *x=levelSearch(p1,BaseName,'D');
		traverseChild(x);
		return;
	}
}

//pwd command
void pwd(){
	if(strcmp(cwd->name,"/")==0){
		printf("/\n");
		return;
	}
	rpwd(cwd);
	printf("\n");
}

//create command
void creat(char *pathname){
	if(strcmp(pathname, "/")==0){
		printf("sorry, can't creat /\n");
		return;
	}
	char temp[128]={'/'};
	//absolute or relative 
	(pathname[0]=='/')?strcpy(temp,pathname):strcat(temp,pathname);	
	strcpy(DirName,dirname(temp));
	printf("dirname = %s ",DirName);
	strcpy(BaseName,basename(temp));
	printf("basename = %s\n",BaseName);	
	printPath(DirName);			
	NODE *p1=iterativeLevelSearch(root, DirName, 'D');
	if(p1!=NULL){
		NODE *x=levelSearch(p1,BaseName, 'F');
		if(x==NULL){
			insert(p1, BaseName,'F');
			printf("---- create OK ----\n");
		}
		else{
			printf("\n---- create : %s already exits\n",BaseName);
		}
	}		
}

//rm command
void rm(char *pathname){
	if(strcmp(pathname, "/")==0){
		printf("pathname=/ : n = 0  ==>\n");
		printf("sorry, can't mkdir /\n");
	}
	//absolute or relative location
	char temp[128]={'/'};
	(pathname[0]=='/')?strcpy(temp,pathname):strcat(temp,pathname);
	strcpy(DirName,dirname(temp));
	printf("dirname = %s  ",DirName);
	strcpy(BaseName,basename(temp));
	printf("basename = %s\n",BaseName);	
	printPath(DirName);			
	NODE *p1=iterativeLevelSearch(root, DirName, 'D');
	if(p1!=NULL){
		NODE *x=levelSearch(p1,BaseName,'F');
		if(x==NULL){
			printf("can't find %s\n",BaseName);
			printf("\n---- rmfile : no such dir ----\n");
		}
		else if(x->node_type!='F'){
			printf("%s : non-DIR\n",BaseName);
		}
		else if(checkEmpty(x)==1){
			printf("---- rmfile : dir not empty ----\n");
		}
		else{
			delete(x);
			printf("\n---- rmfile OK ----\n");
		}
	}		
}

void save(){
	FILE *fp=fopen("myfile1","w+");
	printf("type   path\n");
	printf("---- -------------\n");
	savePreOrder(root, fp);
	fclose(fp);
}

//reload command
void reload(){
	printf("***Start reloading***\n");
	FILE *fp=fopen("myfile1","r");
	char *type;
	char* pathname;
	if(fp==NULL){
		printf("Error: no such file to reload\n");
		return;
	}
	const size_t size=256;
	char* line=malloc(size);
	while(fgets(line,size,fp)!=NULL){
		//printf("line:%s len:%d\n",line,strlen(line));
		type=strtok(line," ");
		printf("type:%s\n",type);
		//sscanf(line,"%c %s", type, pathname);
		pathname=strtok(NULL,"\n");
		printf("pathname:%s\n",pathname);
		if(type=='F'){
			creat(pathname);
		}
		else{
			mkdir(pathname);
		}
	}
	free(line);
	fclose(fp);
}

//menu command
void menu(){
	printf("====================== MENU ========================\n");
	printf("mkdir rmdir cd ls pwd creat rm save reload quit\n");
	printf("====================================================\n");
}

//save the file system tree, then terminate the program
void quit(){
	save();
	exit(0);
}
