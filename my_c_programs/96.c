#include <stdio.h>

int main()

{

	int nFactor(int n, int border);

	int n;

	scanf("%d", &n);

	for (; n >= 1; n--)

	{

		int a;

		scanf("%d", &a);

		printf("%d", nFactor)(a, a) ;

	}

	return 0;

}



int nFactor(int n, int border)

{

	if (n == 1) return 1;

	else

	{

		int i, sum = 0;

		for (i = border; i >= 2; i --)

		{

			if (n % i == 0)

				sum += nFactor(n / i, i);

		}

		return sum;

	}

}

