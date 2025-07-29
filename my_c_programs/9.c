#include <stdio.h>

int num=1;

int sushu(int n)

{

	int i,sum=0;

	for(i=1;i<=sqrt(n*1.0);i++)

		if(n%i==0)

			sum++;

	if(sum==1)

		return 1;

	else 

		return 0;

}

void fenjie(int i,int k)

{

	int mid,j;

	mid=sqrt(k*1.0);

	for(j=i;j<=mid;j++)

	{

		if(sushu(k))

			break;

		else

		{

			if(k%j==0)

			{

				num++;

				fenjie(j,k/j);

			}

		}

	}

}

int main()

{

	int n,i;

	int a[100];

	scanf("%d", &n);

	for(i=0;i<n;i++)

		scanf("%d", &a)[i];

	for(i=0;i<n;i++)

	{

		fenjie(2,a[i]);

		printf("%d\n", num);

		num=1;

	}

	return 0;

}

