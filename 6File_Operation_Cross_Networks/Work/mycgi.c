#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>

#define MAX 10000
#define BLKSIZE 4096

typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];
char buf[4096];

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
    printf("<p>");
  }
  closedir(dir);
}

main(int argc, char *argv[]) {
  int i, m, r;
  char cwd[128];

  m = getinputs();    // get user inputs name=value into entry[ ]
  getcwd(cwd, 128);   // get CWD pathname

  printf("Content-type: text/html\n\n");
  printf("<p>pid=%d uid=%d cwd=%s\n", getpid(), getuid(), cwd);
  printf("<H1>Echo Your Inputs</H1>");
  printf("You submitted the following name/value pairs:<p>");

  for(i=0; i <= m; i++){
    printf("%s = %s<p>", entry[i].name, entry[i].value);
  } 
  printf("<p>");
  /*****************************************************************
   Write YOUR C code here to processs the command

         mkdir dirname
         rmdir dirname
         rm    filename
         cat   filename
         cp    file1 file2
         ls    [dirname] <== ls CWD if no dirname
  *****************************************************************/

  if(strcmp(entry[0].value, "mkdir")==0){
    if (mkdir(entry[1].value, 0777) < 0){
      printf("directory already exist!</p><p>");
    }
    printf("mkdir done</p><p>");
  }

  else if(strcmp(entry[0].value, "rmdir")==0){
    if (rmdir(entry[1].value)<0){
      printf("directory not be find!</p><p>");
    }
    printf("rmdir done</p><p>");
  }
  else if(strcmp(entry[0].value, "rm")==0){
    if(unlink(entry[1].value)!=0){
     printf("file not find!</p><p>");      
    }
    printf("rm done</p><p>");
  }
  else if(strcmp(entry[0].value, "cat")==0){
    FILE *fp;
    int c;
    if (!entry[1].value){
      printf("No file specified.</p><p>");
    }
    fp = fopen(entry[1].value, "r");
    if(fp==0){
      printf("file is empty");
    }
    while ((c = fgetc(fp)) != EOF){
      putchar(c); 
    }
    fclose(fp);
    printf("cat done</p><p>");
  }
  else if(strcmp(entry[0].value, "cp")==0){
    int n;
    int fd, gd;
    fd = open(entry[1].value, O_RDONLY);
    gd = open(entry[2].value, O_WRONLY|O_CREAT);
    while(n=read(fd, buf, BLKSIZE)){
      write(gd, buf, n);
    }
    printf("cp done</p><p>");
    close(fd);close(gd);
  }
  else if(strcmp(entry[0].value, "ls")==0){
    struct stat mystat, *sp;
    int r;
    char *s;
    char name[1024], cwd[1024];
    if (strcmp(entry[1].value,"")==0) {
      getcwd(cwd, 1024);
      strcpy(name, cwd); strcat(name, "/"); 
      ls_dir(name);
    }
    else{
      s = entry[1].value;
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

    printf("ls done</p><p>");
  }
  else{
    printf("invalid command!</p><p>");
  }

  // create a FORM webpage for user to submit again 
  printf("</title>");
  printf("</head>");
  printf("<body bgcolor=\"#B3F1F1\" link=\"#330033\" leftmargin=8 topmargin=8");
  printf("<p>------------------ DO IT AGAIN ----------------\n");
  printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~chen/cgi-bin/mycgi\">");

  //------ NOTE : CHANGE ACTION to YOUR login name ----------------------------
  //printf("<FORM METHOD=\"POST\" ACTION=\"http://cs360.eecs.wsu.edu/~YOURNAME/cgi-bin/mycgi\">");
  
  printf("Enter command : <INPUT NAME=\"command\"> <P>");
  printf("Enter filename1: <INPUT NAME=\"filename1\"> <P>");
  printf("Enter filename2: <INPUT NAME=\"filename2\"> <P>");
  printf("Submit command: <INPUT TYPE=\"submit\" VALUE=\"Click to Submit\"><P>");
  printf("</form>");
  printf("------------------------------------------------<p>");

  printf("</body>");
  printf("</html>");
}
