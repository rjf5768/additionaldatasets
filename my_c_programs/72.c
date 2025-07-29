#include <stdio.h>

int qq(int m){

    int i,j;

    j=0;

    for(i=2;i*i<=j;i++){

     if(m%i==0){

      j=1;

     break;

     }

     }

     return j;

     }

     

int pp(int m,int j){

   int i,k=0;

   if(m==1)

   return 1;

   else {

   k++;

   for(i=j;i*i<=m;i++){

    if(qq(i)==0&&m%i==0){

     k=k+pp(m/i,i);

     }

     }

   }  

  return k;   

}

      

int main(){

    int n,i,p;

    scanf("%d", &n);

    int aa[n+1];

    for(i=1;i<=n;i++){

    scanf("%d", &p);

    aa[i]=pp(p,2);

}

   for(i=1;i<=n;i++)

   printf("%d\n", aa[i]);



    return 0;

}