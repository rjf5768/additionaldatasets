#include <stdio.h>

/*

 * ????1118-4.cpp

 * ?????

 * ????: 2011-11-29

 * ??????????a?????????????????a = a1 * a2 * a3 * ... * an???1 < a1 <= a2 <= a3 <= ... <= an?????????????

 */

int divide(int n,int a)

{

	int s=0,i;

	if(n==1) return 1;

	for(i=a;i>1;i--)

		if(n%i==0)

			s+=divide(n/i,i);

	return s;

}

int main()

{

	int n,k;

	scanf("%d", &k);

	while(k>0)

	{

		scanf("%d", &n);

		printf("%d\n", divide(n,n));

		k--;

	}

	return 0;

}

