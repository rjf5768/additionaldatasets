#include <stdio.h>

int num;

void divide(int n,int n2)

{

	int i,m1;

	m1=(int)sqrt((double)n);

	for(i=n2;i<=m1;i++)

	{if(n%i==0)

	{num++;divide(n/i,i);}

	}

}

int main()

{

	int n,i,m;

	scanf("%d", &n);

	for(i=0;i<n;i++)

	{

		num=1;

		scanf("%d", &m);

		

		divide(m,2);

		printf("%d\n", num);

	}

	return 0;

}