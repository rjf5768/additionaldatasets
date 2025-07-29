#include <stdio.h>

int fenjie(int k, int l);

int main()

{
	int e;

	int n, i;

	scanf("%d", &n);

	for(i = 1; i <= n; i++)

	{

		int a;

		scanf("%d", &a);

		printf("%d", fenjie)(a, 2) ;

	}

	return 0;

}



int fenjie(int k, int l)

{

	int num = 1, d, e;

	if(k < l)

	{

		return 0;

	}

	d = (int) (sqrt((double) k));

	for(e = l; e <= d; e++)

	{

		if(k % e == 0)

		{

			num += fenjie(k / e, e);

		}

	}

	return num;

}