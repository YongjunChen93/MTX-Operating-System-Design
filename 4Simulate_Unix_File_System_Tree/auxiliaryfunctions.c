#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include "auxiliaryfunctions.h"

/*-------------- Auxiliary Functions ----------------*/
int checkEmpty(NODE *target){
	if(target->childPtr!=NULL){
		return 1;//not empty
	}
	return 0;//empty
}

void traverseChild(NODE *parent){
	NODE *cur=parent->childPtr;
	if(cur==NULL){	
		return;//won't print anything if list that directory(no child)
	}
	while(cur!=NULL){		
		printf("%c  %s\n",cur->node_type,cur->name); 
		cur=cur->siblingPtr;
	}
}

//one-level search-later in the mkdir function recursively call this one-level search by changing cur
//if a cur node has a child whose name==name,type== then return the child/sibling
//type='D'
NODE *levelSearch(NODE* cur, char name[64], char type){
	printf("Search for %s in %s: ", name, cur->name);
	NODE *child = cur->childPtr;	
	if(child==NULL)
		return NULL;
	while(child!=NULL){
		printf("%s \n",child->name);
		if(strcmp(child->name,name)==0 && child->node_type==type){
			return child;
		}
		child=child->siblingPtr;
	}
	return NULL;	
}

//iterative level-search NODE *levelSearch(NODE* cur, char name[64], char type)
NODE *iterativeLevelSearch(NODE *cur, char directoryName[64], char type){
	if(strcmp(DirName,"/")==0){
		return cur;
	}
	//first token
	char *token;
	char dircopy[64];
	printf("directory---->%s\n",directoryName);
	token=strtok(directoryName, "/"); 
	printf("token ------>>>>%s\n",token);
	NODE *c=levelSearch(cur, token, type);
	if(c==NULL){
		printf("can't find %s\n", token);	
		return NULL;
	}
	NODE *newStarter=c;	
	//get the token until it returns NULL pointer
	while(token=strtok(0,"/")){
		c=levelSearch(newStarter, token, type);
		if(c==NULL){
			printf("can't find %s\n", token);
			return NULL;
		}
		newStarter=c;
	}
	return c;
}

// for pwd
void rpwd(NODE *P){
	char s[256];
	if(strcmp(P->name,"/")==0){
		return;
	}	
	rpwd(P->parentPtr);
	printf("/%s",P->name);
	return;
}

void delete(NODE *target){
	NODE* parent=target->parentPtr;
	//guarantee no child-->can be deleted
	if(parent->childPtr==target){
		parent->childPtr=target->siblingPtr;
	}
	//target is not the oldest child
	else{
		NODE *cur=parent->childPtr;
		while(cur->siblingPtr!=target && cur->siblingPtr!= NULL){
			cur=cur->siblingPtr;
		}
		cur->siblingPtr=target->siblingPtr;
	}
}

int countToken(char *dir){
	char copy[64];
	strcpy(copy,dir);
	char *s;
	int count=0;
	char *p;
	for(p=strtok(copy,"/");p!=NULL;p=strtok(NULL,"/")){
		count++;
	}
	return count;
}

void printPath(char *dir){	
	printf("pathname = %s : n = %d ==> ",dir, countToken(dir));
	char *p;
	char dircopy[64];
	strcpy(dircopy,dir);
	for(p=strtok(dircopy,"/");p!=NULL;p=strtok(NULL,"/")){
		printf("%s ",p);
	}
	printf("\n");
}

// insert function
void insert(NODE *parent, char NewName[64], char NewType){
	printf(" Insert %s\n", NewName);
	NODE *newNode=(NODE*)malloc(sizeof(NODE));
	newNode->node_type=NewType;
	strcpy(newNode->name, NewName);
	newNode->childPtr=NULL;
	newNode->siblingPtr=NULL;
	newNode->parentPtr=parent;
	if(parent->childPtr==NULL){
		parent->childPtr=newNode;
	}
	else{
		NODE *pos=parent->childPtr;
		while(pos->siblingPtr!=NULL){
			pos=pos->siblingPtr;
		}
		pos->siblingPtr=newNode;
	}
}

// for save
char* getParentPath(NODE *cur){
	char result[128];
	if(cur==root){
		strcpy(result,"");
		return result;
	}
	if(cur->parentPtr==root){
		strcpy(result,"/");
		return result;
	}		
	strcpy(result,"/");
	while(cur->parentPtr!=root){
		cur=cur->parentPtr;
		strcat(result, cur->name);
		strcat(result,"/");
	}	
	//reverse(result);
	return result;	
}

void savePreOrder(NODE *ROOT, FILE *fp){
	char s[256];
	if(ROOT==NULL){
		return;
	}	
	strcpy(s,getParentPath(ROOT));
	fprintf(fp,"%c      %s%s\n",ROOT->node_type,s, ROOT->name);
	printf("%c      %s%s\n",ROOT->node_type,s, ROOT->name);
	savePreOrder(ROOT->childPtr,fp);
	if(ROOT!=root){
		savePreOrder(ROOT->siblingPtr,fp);
	}	
} 

