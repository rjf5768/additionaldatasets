#include <stdio.h>













int decompose(int x, int y);

int main ()

{

	int n, m;

	scanf("%d", &n);

	for(j=1; j <= n; j++)

	{

		scanf("%d", &m);		

		printf("%d", decompose)(m, 2) ;

	}

	return 0;

}

int decompose(int x, int y)

{

	int sum = 1,b;

	b = (int)sqrt(x);

	if (x == 1) return 0;

	for(i=y; i <= b; i++)

	{

		if (x % i == 0)

		{	

			 sum = sum + decompose(x / i, i);

		}

	}

	return sum;

}

