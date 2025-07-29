#include <stdio.h>

int cal; 

int fac(int a,int x){ 

if(x==1) { cal++; return 1; } 

for(i=2;i<=x;i++)

if(x%i==0&&i>=a) 

fac(i,x/i);}

int main(){

int n,b;

scanf("%d", &n);

while(n>0){

cal=0;

scanf("%d", &b);

fac(1,b);

printf("%d\n", cal);

n--;

}}