#include <stdio.h>



/**

* @author ??

* @date 2010-12-1

* @description

* ??????:???? 

*/

int sum;

void calculate(int, int);





int main()

{

	int n;

	scanf("%d", &n);

	for(i=0; i < n;i++)

	{

		int m;

		sum  = 1;

		scanf("%d", &m);

		calculate(m, 2);

		printf("%d\n", sum);

	}

	return 0;

}

void calculate(int m,int begin)

{

	int i;

	int n = sqrt((double)m);

	for ( i = begin; i <= n; i++ )

	{

		if ( m % i == 0)

		{

			sum++;

			calculate(m / i,i);

		}

	}

}