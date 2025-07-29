#include <stdio.h>

int sum=0;

int main()

{

	void yinshu(int,int);

	int i,n,a;

	scanf("%d", &n);

	for(i=1;i<=n;i++)

	{

		sum=0;

		scanf("%d", &a);

		yinshu(a,2);

		printf("%d\n", sum);

	}

	return 0;

}



void yinshu(int a,int x)

{

	if(a!=1)

	{

	for(i=x;i<=a;i++)

		if(a%i==0)

			yinshu(a/i,i);

	}

	if(a==1)

		sum++;

}