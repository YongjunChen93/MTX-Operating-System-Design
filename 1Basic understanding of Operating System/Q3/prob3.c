int *FP; // a global pointer

int B(int x, int y)
{
int u,v,w;
int *p;

printf("enter B\n");
u=7; v=8; w=9;

p = (int  *)&p;
asm("movl %ebp, FP"); // set FP=CPU’s %ebp register
printf("FP = %8x p =%8x\n", FP, p);

while(FP){
printf("%8X->", FP);
FP= (int *)*FP;
}
printf("NULL\n");

getchar();

// Write C code to DO (1)-(3) AS SPECIFIED BELOW
for(int i=0;i<150;i++)
{
printf("%8x\t%8x\n",(int)p,(int)*p);
p++;
}
printf("---------------------------------------------\n");
printf("exit B\n");
}

int A(int x, int y)
{
int d,e,f;
printf("enter A\n");
printf("&d=%x &e=%x &f=%x\n", (int)&d, (int)&e, (int)&f);
d=4; e=5; f=6;
B(d,e);
printf("exit A\n");
}

main(int argc, char *argv[], char *env[])
{
int a,b,c;
printf("enter main\n");
printf("====argc, argv, env===\n");
printf("&argc=%x\n",(int)&argc);
for(int i=0; i< argc; i++){
printf("argv[%x] = %s address = %x\n",i,argv[i],(int)&argv[i]);
}
int j=0;
while(env[j]){
printf("env[%x] address = %x\n",j,(int)&argv[j]);
j++;
}
a=1; b=2; c=3;
A(a,b);
printf("exit main\n");
}
