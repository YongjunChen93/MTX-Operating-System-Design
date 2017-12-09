
int my_cat(char *pathname){
	char mybuf[BLKSIZE];
	int dummy = 0; //a null char at end of mybuf[ ]
	int n = 0;
	strcpy(parameter,"0");
	//1. open file for read
	int fd = my_open(pathname, 0);
	if(fd == -1){
		printf("cat: file open error\n");
		return -1;
	}
	//2. read from file
	printf("============cat file===================\n");
	//print char from buf one by one
	while(n = myread(fd, mybuf, 1024,0)){
		mybuf[n] = '\0';
		int i = 0;
		while(mybuf[i] != '\0'){
			if(mybuf[i] == '\n'){
				putchar('\r');
			}
			putchar(mybuf[i]);
			i++;
		}
	}
	printf("==============cat file done==================\n");
	my_close(fd);
	return 0;
}

int my_cp(char *pathname, char *parameter){
	int n = 0;
	char buf[BLKSIZE];
	//1 fd = open src for R
	int src_fd = my_open(pathname, 0);
	if(src_fd == -1){
		printf("bad SRC file\n");
	}
	//2 gd= open dst for W
	int dest_dev = 0;
	if(parameter[0]=='/'){
		dest_dev = root->dev;
	}
	else{
		dest_dev = running->cwd->dev;
	}
	int dest_ino = getino(&dest_dev, parameter);
	//not such file, creat it
	printf("++++++++++++++%s\n",parameter);
	if(dest_ino == 0){
		my_creat(parameter);
	}
	int des_gd = my_open(parameter, 2);
	while(n = myread(src_fd, buf, BLKSIZE, 0)){
		mywrite(des_gd, buf, n);
	}
	my_close(src_fd);
	my_close(des_gd);
	return 0;
}

int my_mv(char *pathname, char *parameter){
	int src_dev = 0;
	//src part
	if(pathname[0] == '/'){
		src_dev = root->dev;
	}
	else{
		src_dev = running->cwd->dev;
	}
	int src_ino = getino(&src_dev, pathname);
	if(src_ino == 0){
		printf("the source file doesn't exist.\n");
		return -1;
	}
	MINODE *src_mip = iget(src_dev, src_ino);
	// dest part
	int dest_dev = 0;
	if(parameter[0] == '/'){
		dest_dev = root->dev;
	}
	else{
		dest_dev = running->cwd->dev;
	}

	my_cp(pathname, parameter);
	my_unlink(pathname);
	return 0;
}

