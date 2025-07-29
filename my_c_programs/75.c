#include <stdio.h>

void account (int ,int);

int p;

int main()

{

	int m;

	int n;

	scanf("%d", &n);

	while(n>0)

	{

		p=0;

    scanf("%d", &m);

account(m,1);

printf("%d\n", p);

n--;

	}

return 0;

}

void account (int m,int i)

{

if(m==1)

    p++;

else for(j=2;j<=m;j++)

	{

	if((m%j==0)&&(j>=i))

     account(m/j,j);

	}

}