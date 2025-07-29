#include <stdio.h>

/*

 * e.cpp

 * ?????

 * ????: 2010-12-8

 * ???????

 */



int find_factor(int a,int b)

{

	int i,sum=0;

	if(a==1) return 1;

	for(i=b;i<=a;i++)

		if(a%i==0)

			sum=sum+find_factor(a/i,i);

	return sum;

}



int main(void)

{

	int n;

	scanf("%d", &n);

	int a;

	for(i=1;i<=n;i++)

	{

		scanf("%d", &a);

		printf("%d\n", find_factor(a,2));

	}

	return 0;

}