// This is the echo SERVER server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>


#define  MAX 256
#define BLKSIZE 256

// Define variables:
struct sockaddr_in  server_addr, client_addr, name_addr;
struct hostent *hp;

int  mysock, client_sock;              // socket descriptors
int  serverPort;                     // server port number
int  r, length, n;                   // help variables

//ls
struct stat mystat, *sp;

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

int ls_file(char *line, char *fname){
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];
  strcat(line, fname);
  strcat(line, "\t");
  sp = &fstat;
  if ( (r = lstat(fname, &fstat)) < 0){
     printf("can't stat %s\n", fname); 
     exit(1);
  }
  if ((sp->st_mode & 0xF000) == 0x8000)
    strcat(line,"-");
  if ((sp->st_mode & 0xF000) == 0x4000)
    strcat(line,"d");
  if ((sp->st_mode & 0xF000) == 0xA000)
    strcat(line,"l");
  char buffer[256];

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
    strcat(line, ftime);

  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
    char buf[1024];
    readlink(fname, buf, sizeof(buf));
    strcat(line,buf);
  }
  strcat(line, "\n");
}

int ls_dir(char *line, char *dname){
  DIR *dir;
  dir=opendir(dname);
  struct dirent *dp;
  while(dp=readdir(dir)){;
    ls_file(line, dp->d_name);
    printf("ls_file %s\n",dirname(dp->d_name));
  }
  closedir(dir);
}

// Server initialization code:

int server_init(char *name){
    printf("==================== server init ======================\n");   
    // get DOT name and IP address of this host

    printf("1 : get and show server host info\n");
    hp = gethostbyname(name);
    if (hp == 0){
        printf("unknown host\n");
        exit(1);
    }
    printf("    hostname=%s  IP=%s\n",
    hp->h_name,  inet_ntoa(*(long *)hp->h_addr));
    
    //  create a TCP socket by socket() syscall
    printf("2 : create a socket\n");
    mysock = socket(AF_INET, SOCK_STREAM, 0);
    if (mysock < 0){
        printf("socket call failed\n");
        exit(2);
    }

    printf("3 : fill server_addr with host IP and PORT# info\n");
    // initialize the server_addr structure
    server_addr.sin_family = AF_INET;                  // for TCP/IP
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);   // THIS HOST IP address  
    server_addr.sin_port = 0;   // let kernel assign port

    printf("4 : bind socket to host info\n");
    // bind syscall: bind the socket to server_addr info
    r = bind(mysock,(struct sockaddr *)&server_addr, sizeof(server_addr));
    if (r < 0){
        printf("bind failed\n");
        exit(3);
    }

    printf("5 : find out Kernel assigned PORT# and show it\n");
    // find out socket port number (assigned by kernel)
    length = sizeof(name_addr);
    r = getsockname(mysock, (struct sockaddr *)&name_addr, &length);
    if (r < 0){
        printf("get socketname error\n");
        exit(4);
    }

    // show port number
    serverPort = ntohs(name_addr.sin_port);   // convert to host ushort
    printf("    Port=%d\n", serverPort);

    // listen at port with a max. queue of 5 (waiting clients) 
    printf("5 : server is listening ....\n");
    listen(mysock, 5);
    printf("===================== init done =======================\n");
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

main(int argc, char *argv[]){
    char *hostname;
    char line[MAX],splitline[MAX], cwd[128];
    char *entry[MAX];
    if (argc < 2)
        hostname = "localhost";
    else
        hostname = argv[1];

    server_init(hostname); 

    // Try to accept a client request
    while(1){
        printf("server: accepting new connection ....\n"); 

        // Try to accept a client connection as descriptor newsock
        length = sizeof(client_addr);
        client_sock = accept(mysock, (struct sockaddr *)&client_addr, &length);
        if (client_sock < 0){
            printf("server: accept error\n");
            exit(1);
        }
        printf("server: accepted a client connection from\n");
        printf("-----------------------------------------------\n");
        printf("        IP=%s  port=%d\n", inet_ntoa(client_addr.sin_addr.s_addr),
                    ntohs(client_addr.sin_port));
        printf("-----------------------------------------------\n");

        // Processing loop: newsock <----> client
        while(1){
            n = read(client_sock, line, MAX);
            printf("line server%s\n",line);
            if (n==0){
                printf("server: client died, server loops\n");
                close(client_sock);
            break;
            }
            printf("********************** menu *********************\n");
            printf("* get  put  ls   cd   pwd   mkdir   rmdir   rm  *\n");
            printf("* lcat     lls  lcd  lpwd  lmkdir  lrmdir  lrm  *\n");
            printf("*************************************************\n");

            // LAB3 works
            printf("server: read  n=%d bytes; line=[%s]\n", n, line);

            //split command into array
            strcpy(splitline, line);
            decompose(splitline, " ", entry);
            strcpy(line, "");
            // execute command
            if(strcmp(entry[0], "get")==0){
                char buf[MAX];
                struct stat st;
                stat(entry[1], &st);
                int size = st.st_size;
                snprintf(buf,MAX,"%d",size); 
                write(client_sock, buf,MAX);
                int m = 0;
                int fd = open(entry[1],O_RDONLY);
                while(m = read(fd, buf, MAX)){
                    write(client_sock, buf, m);
                }
                close(fd);
                printf("donload %sto client\n",entry[1]);
            }
            else if(strcmp(entry[0], "put")==0){
                char buf[MAX];
                n = read(client_sock, buf, MAX);
                n = read(client_sock, buf, MAX);
                int SIZE;
                SIZE = atoi(buf);
                if(SIZE<0){
                    break;
                }
                int m=0;
                int fd = open(entry[1],O_WRONLY|O_CREAT);
                int count =0;
                while(count < SIZE){
                    m = read(client_sock, buf, MAX);
                    write(fd, buf, m);
                    count += m;
                }
                close(fd);
                printf("upload %s from client.\n",entry[1]);
            }
            else if(strcmp(entry[0], "ls")==0){
                struct stat mystat, *sp;
                int r;
                char *s;
                char name[1024], cwd[1024];
                if (entry[1]==NULL){
                    getcwd(cwd, 1024);
                    strcpy(name, cwd); strcat(name, "/"); 
                    ls_dir(line, name);
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
                        ls_dir(line, name);
                    }
                    else{
                        ls_file(line, name);
                    }
                }
                // send the echo line to client 
                n = write(client_sock, line, MAX);
            }
            else if(strcmp(entry[0], "cd")==0){
                char cwd[1024];
                if(entry[1]!= NULL){
                    chdir(entry[1]);
                    getcwd(cwd, 128);
                }
                else{
                    chdir("/");
                    getcwd(cwd, 128);
                }
                strcpy(line, "cd done");
                printf("cd done\n");
                // send the echo line to client 
                n = write(client_sock, line, MAX);
            }

            else if(strcmp(entry[0], "pwd")==0){
                char cwd[1024];
                getcwd(cwd, 1024);
                strcpy(line, cwd);
                printf("pwd = %s\n",cwd);
                // send the echo line to client 
                n = write(client_sock, line, MAX);
            }
            else if(strcmp(entry[0], "mkdir")==0){
                if (mkdir(entry[1], 0777) < 0){
                    printf("truee mkdir ===---");
                    strcpy(line,"directory already exist!");
                }
                strcpy(line,"mkdir done!");
                printf("mkdir done\n");
                // send the echo line to client 
                n = write(client_sock, line, MAX);
            }

            else if(strcmp(entry[0], "rmdir")==0){
                if (rmdir(entry[1])<0){
                    strcpy(line, "directory not be find!");
                }
                strcpy(line, "rmdir done");
                printf("rmdir done\n");
                 // send the echo line to client 
                n = write(client_sock, line, MAX);
            }

            else if(strcmp(entry[0], "rm")==0){
                if(unlink(entry[1])!=0){
                    strcpy(line, "file not find!");      
                }
                strcpy(line, "rm done");
                printf("rm done\n");
                 // send the echo line to client 
                n = write(client_sock, line, MAX);
            }

            else{
                strcpy(line,"invalid command!");
                printf("invalid command!\n");
                 // send the echo line to client 
                n = write(client_sock, line, MAX);
            }

            printf("server: ready for next request\n");
        }
    }
}

