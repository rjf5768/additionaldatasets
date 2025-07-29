#include <stdio.h>

int begin=2;

int main()

{

	int f(int);

	int n=0,x=0,i=0;

	scanf("%d", &n);

	for(i=0;i<n;i++)

	{

		scanf("%d", &x);

		printf("%d", f)(x)+1;

	}

	return 0;

}

int f(int x)

{

	int i=0;

	int left=0;

	int num=0;

	for(i=begin;i<=sqrt(x);i++)

	{

		if(x%i==0)

		{

			num++;

			left=x/i;

			begin=i;

			num+=f(left);

		}

	}

	

	    begin=2;

	    return num;

	

		

}

