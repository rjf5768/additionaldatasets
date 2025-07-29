#include <stdio.h>



int dp[40000][200]={0};

int main()

{

    int n,i,N;

    int mm( int , int );

    

    scanf("%d", &n);

    for(i=1;i<=n;i++)

    {

                     scanf("%d", &N);

                     printf("%d\n", mm(N,2));

                   

    }

   

    return 0;

}



int mm( int n, int i)

{

    int sum=1;

    

    if(dp[n][i]>0) return dp[n][i];

    

    

    for(j=i; j<=sqrt(n);j++)

    {

             if(n%j==0)

             {

                       sum += mm(n/j,j);

             }

    }

    dp[n][i]=sum;

    return sum;

}

    

    

    

    

    

    

    

    

    

    

    

    

    

    

    

    

    

    

    

    

    

  

