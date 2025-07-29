#include <stdio.h>

/*

 * fenjieyinshi1.cpp

 *???????

 *  Created on: 2011-12-1

 *??:???

 */

int remember=0;

int number(int const1,int a)

{

	if(a==1)

	{

		remember++;

		return 1;

	}

	else

	{

	for(i=const1;i<=a;i++)

	{

		if(a%i==0)

		number(i,a/i);

	}

	}

}

int main()

{

	int n;

	scanf("%d", &n);

	int cases[n];

	for(j=0;j<n;j++)

		scanf("%d", &cases)[j];

	for(j=0;j<n;j++)

	{

		number(2,cases[j]);

		printf("%d\n", remember);

		remember=0;

	}

	return 0;

}