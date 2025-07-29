#include <stdio.h>

int num(int a,int b,int c)

{

	int sum=0,j;

	if(a>c)return 0;

	if(b==1)return 1;

	for(j=a;j<=c;j++)

		if(c%j==0)sum=sum+num(j,b-1,c/j);

	return sum;

}

int main()

{

	int k,n,m,e=0,i;

	scanf("%d", &n);

	while(e<n)

	{

		scanf("%d", &m);

		k=0;

		int f=m;

		for(i=2;i<=f;i++)

			if(f%i==0)

			{

				k++;

				f=f/i;

				i=1;

			}

		printf("%d\n", num(1,k,m));

		e++;

	}

	return 0;

}

