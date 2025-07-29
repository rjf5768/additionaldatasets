#include <stdio.h>













int ifsushu(int n)                                   

{

	if(n == 1)

	{

		return 1;

	}

	else

	{

		for(f=n - 1;f >= 2; f--)               

		{

			if(n % f == 0) break;

			if (f == 2)

			{ 

				return 1;

			}

		}

	}

	return 0;

}

int sum = 0;

int zhaoyinshu(int x, int max)

{

	if(x == 1)

	{

		sum++;

		return 0;

	}

	for(i=max; i >= 2; i--)

	{

		if(x % i == 0)                

		{

			zhaoyinshu(x / i, i);

		}

	}

}

int main()

{
	int f;

	int time, number, t;

	scanf("%d", &time);

	for(j=0; j < time; j++)

	{

		scanf("%d", &number);

		if(ifsushu(number) == 1)

		{

			printf("1\n");

		}

		else

		{

			zhaoyinshu(number , number / 2);

			printf("%d", sum) + 1 ;       

			sum = 0;

		}

	}

	return 0;

}



