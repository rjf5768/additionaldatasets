#include <stdio.h>

int s(int n,int m){

if (n==1)return 1;

int an=0;

for(i=m;i>1;i--){

if (n%i==0)an+=s(n/i,min(i,n/i));

}

return an;

}

int main(){

int N;

scanf("%d", &N);

int x;

for (N;N>0;N--){

scanf("%d", &x);

printf("%d\n", s(x,x));

}

return 0;

}