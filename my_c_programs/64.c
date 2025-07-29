#include <stdio.h>

int num = 0;

void check(int m, int n)

{

	for(;m <= n; m++)

	{

		if(n % m == 0)

		{

			if(n / m == 1)

			{	

				num++;

				return;

			}

			else

				check(m, n / m);

		}

	}

}

int main()

{

	int n, m;

	scanf("%d", &n);

	for(; n; n--)

	{

		scanf("%d", &m);

		check(2, m);

		printf("%d\n", num);

		num = 0;

	}

	return 0;

}