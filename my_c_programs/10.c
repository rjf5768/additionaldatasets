#include <stdio.h>





int num;

int dg(int m,int limit);



int main()

{

    int n;

    scanf("%d", &n);

    for(i=1;i<=n;i++)

    {

        int m; scanf("%d", &m);

        num=0;

        dg(m,2);

        printf("%d\n", num);

    }

    return 0;

}

int dg(int m,int limit)

{

    for(i=limit;i<=m/2;i++)

        if (m%i==0) dg(m/i,i);

    if (m>=limit) num++;

    return 0;

}



