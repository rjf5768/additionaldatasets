#include <stdio.h>

int main()

{

  int fj(int m,int min); 

  int t,i,a[100];

  scanf("%d", &t);

  for(i=1;i<=t;i++)

     {

       

       scanf("%d", &a)[i];

       printf("%d\n", fj(a[i],2));

     }

 

  return 0;

}

   int fj(int m,int min)

{

   int counter=1;

   int i;

   if(m<min)

     return 0;

     for(i=min;i<m;i++)

         if(m%i==0)

         {

           counter=fj(m/i,i)+counter; 

         }

   return counter;

   

}