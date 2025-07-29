#include <stdio.h>

int num;

int main()

{

    int f(int,int);  

    int i,n,a[100],result[100];

    scanf("%d", &n);

 

    for(i=1;i<=n;i++)

    {

                     num=1; 

                     scanf("%d", &a)[i];

                     result[i]=f(1,a[i]);

    }

    for(i=1;i<=n;i++)

    {

                     printf("%d\n", result[i]);

    }

}

int f(int x,int y)

{

    int i;

    for(i=2;i<=sqrt(y);i++) 

    {

                           if(y%i==0&&i>=x)

                           {

                                     num++;

                                     f(i,y/i); 

                           }

    }

    return num;  

} 

