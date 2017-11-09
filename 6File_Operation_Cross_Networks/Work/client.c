// The echo client client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netdb.h>


#define MAX 256
#define BLKSIZE 256
// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr; 

int server_sock, r;
int SERVER_IP, SERVER_PORT; 

char buf[256];

//ls
struct stat mystat, *sp;

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int ls_file(char *fname){
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];

  sp = &fstat;
  if ( (r = lstat(fname, &fstat)) < 0){
     printf("can't stat %s\n", fname); 
     exit(1);
  }
  if ((sp->st_mode & 0xF000) == 0x8000)
     printf("%c",'-');
  if ((sp->st_mode & 0xF000) == 0x4000)
     printf("%c",'d');
  if ((sp->st_mode & 0xF000) == 0xA000)
     printf("%c",'l');
  for (i=8; i >= 0; i--){
    if (sp->st_mode & (1 << i))
      printf("%c", t1[i]);
    else
      printf("%c", t2[i]);
  }

  printf("%4d ",sp->st_nlink);
  printf("%4d ",sp->st_gid);
  printf("%4d ",sp->st_uid);
  printf("%8d ",sp->st_size);

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  printf("time==%s  ",ftime);
  // print name
  printf("%s", basename(fname));  

  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
    char buf[1024];
    readlink(fname, buf, sizeof(buf));
    printf(" -> %s", buf);
  }
  printf("\n");
}

int ls_dir(char *dname){
  DIR *dir;
  dir=opendir(dname);
  struct dirent *dp;
  while(dp=readdir(dir)){;
    ls_file(dp->d_name);
  }
  closedir(dir);
}

// clinet initialization code

int client_init(char *argv[])
{
    printf("======= clinet init ==========\n");
    printf("1 : get server info\n");
    hp = gethostbyname(argv[1]);
    if (hp==0){
        printf("unknown host %s\n", argv[1]);
        exit(1);
    }

    SERVER_IP   = *(long *)hp->h_addr;
    SERVER_PORT = atoi(argv[2]);
    printf("2 : create a TCP socket\n");
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock<0){
        printf("socket call failed\n");
        exit(2);
    }

    printf("3 : fill server_addr with server's IP and PORT#\n");
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = SERVER_IP;
    server_addr.sin_port = htons(SERVER_PORT);

    // Connect to server
    printf("4 : connecting to server ....\n");
    r = connect(server_sock,(struct sockaddr *)&server_addr, sizeof(server_addr));
    if (r < 0){
        printf("connect failed\n");
        exit(1);
    }

    printf("5 : connected OK to \007\n"); 
    printf("---------------------------------------------------------\n");
    printf("hostname=%s  IP=%s  PORT=%d\n", 
        hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
    printf("---------------------------------------------------------\n");
    printf("========= init done ==========\n");
}

void decompose(char *input, char* token, char** array){
    int index=0;
    char *p=strtok(input,token);
    while(p!=NULL){
        array[index++]=p;
        p=strtok(NULL,token);
    }
    array[index]=NULL;
}

main(int argc, char *argv[ ])
{
    int n, c;
    char line[MAX], ans[MAX], splitline[MAX],line2[MAX];
    char *entry[MAX];
    if (argc < 3){
        printf("Usage : client ServerName SeverPort\n");
        exit(1);
    }
    client_init(argv);
    // sock <---> server
    printf("********  processing loop  *********\n");
    while (1){
        printf("********************** menu *********************\n");
        printf("* get  put  ls   cd   pwd   mkdir   rmdir   rm  *\n");
        printf("* lcat     lls  lcd  lpwd  lmkdir  lrmdir  lrm  *\n");
        printf("*************************************************\n");

        printf("input a line : ");
        bzero(line, MAX);                // zero out line[ ]
        fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

        line[strlen(line)-1] = 0;        // kill \n at end
        if (line[0]==0)  exit(0);                // exit if NULL line
    
        //LAB 3 Start
        //split command into array
        strcpy(splitline, line);
        strcpy(line2, line);
        decompose(splitline, " ", entry);
        //quit
        if(strcmp(entry[0], "quit")==0){
            exit(1);
        }
        // local command
        if(strcmp(entry[0], "lmkdir")==0){
            if (mkdir(entry[1], 0777) < 0){
                printf("directory already exist!");
            }  
            printf("mkdir done");
        }

        else if(strcmp(entry[0], "lrmdir")==0){
            if (rmdir(entry[1])<0){
                printf("directory not be find!");
            }
            printf("rmdir done");
        }

        else if(strcmp(entry[0], "lrm")==0){
            if(unlink(entry[1])!=0){
                printf("file not find!");      
            }
            printf("rm done");
        }

        else if(strcmp(entry[0], "lcat")==0){
            FILE *fp;
            int c;
            if (!entry[1]){
                printf("No file specified.");
            }
            fp = fopen(entry[1], "r");
            if(fp==0){
                printf("file is empty");
            }
            while ((c = fgetc(fp)) != EOF){
                putchar(c); 
            }
            fclose(fp);
        }

        else if(strcmp(entry[0], "lpwd")==0){
            char cwd[1024];
            getcwd(cwd, 1024);
            printf("%s\n", cwd);
            
        }

        else if(strcmp(entry[0], "lcp")==0){
            int n;
            int fd, gd;
            fd = open(entry[1], O_RDONLY);
            gd = open(entry[2], O_WRONLY|O_CREAT);
            while(n=read(fd, buf, BLKSIZE)){
                write(gd, buf, n);
            }
            close(fd);close(gd);
        }

        else if(strcmp(entry[0], "lcd")==0){
            char cwd[1024];
            if(entry[1]!= NULL){
                chdir(entry[1]);
                getcwd(cwd, 128);
            }
            else{
                chdir("/");
                getcwd(cwd, 128);
            }
        }

        else if(strcmp(entry[0], "lls")==0){
            struct stat mystat, *sp;
            int r;
            char *s;
            char name[1024], cwd[1024];
            if (entry[1]==NULL){
                getcwd(cwd, 1024);
                strcpy(name, cwd); strcat(name, "/"); 
                ls_dir(name);
            }
            else{
                s = entry[1];
                sp = &mystat;
                if (r = lstat(s, sp) < 0){
                    printf("no such file %s\n", s); 
                }
                strcpy(name, s);
                if (s[0] != '/'){    // name is relative : get CWD path
                    getcwd(cwd, 1024);
                    strcpy(name, cwd); strcat(name, "/"); strcat(name,s);
                }
                if (S_ISDIR(sp->st_mode)){
                    ls_dir(name);
                }
                else{
                    ls_file(name);
                }
            }
        }

        else{
            //get and put
            if(strcmp(entry[0], "get")==0){
                n = write(server_sock, line2, MAX);
                n = read(server_sock, buf, MAX);
                int SIZE;
                SIZE = atoi(buf);
                if(SIZE<0){
                    break;
                }
                int m=0;
                int fd = open(entry[1],O_WRONLY|O_CREAT);
                int count =0;
                while(count < SIZE){
                    m = read(server_sock, buf, MAX);
                    write(fd, buf, m);
                    count += m;
                }
                close(fd);
                printf("download %s from server.\n", entry[1]);
            }
            else if(strcmp(entry[0], "put")==0){
                n = write(server_sock, line2, MAX);
                char buf[MAX];
                struct stat st;
                stat(entry[1], &st);
                int size = st.st_size;
                snprintf(buf,MAX,"%d",size); 
                write(server_sock, buf,MAX);
                int m = 0;
                int fd = open(entry[1],O_RDONLY);
                while(m = read(fd, buf, MAX)){
                    write(server_sock, buf, m);
                }
                close(fd);
                printf("upload %s to server\n",entry[1]);
            }
            else{
                // Send ENTIRE line to server
                n = write(server_sock, line, MAX);
                printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

                // Read a line from sock and show it
                n = read(server_sock, ans, MAX);
                printf("client: read  n=%d bytes; echo=\n%s\n",n, ans);
            }
        }
    }
}


