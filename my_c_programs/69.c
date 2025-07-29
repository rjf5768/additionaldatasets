#include <stdio.h>

/*???2011?11?28?

 *??????

 *????division.cpp

 *????? - 1090 ????

 */





int main()

{
	int a;

	int n = 0, integer = 0;							

	int division(int, int);								

													

	scanf("%d", &n);										

	for(i=0; i < n; i++)

	{

		scanf("%d", &integer);

		printf("%d", division)(integer, integer) ;			

	}

	return 0;

}





int division(int integer, int limit)

{

	if (limit <= 1)

		return 0;

	switch(integer)

	{

	case 1:

		return 1;

		break;

	default:

	{

		int sum = 0;

		for(a=1; a <= integer; a++)

		{

			if (integer%a != 0)

				continue;

			else if (integer/a > limit)

				continue;

			else

				sum += division(a, integer/a);

		}

		return sum;

	}

	break;

	}

}

