#include <stdio.h>

int ways(int n,int p)

{

	if(n<p) return 0;

	while(n%p!=0) p++;

	if(n==p) return 1;

	return ways(n/p,p)+ways(n,p+1);

}

int main()

{

	int n,m;

	scanf("%d", &n);

	for(i=0;i<n;i++)

	{

		scanf("%d", &m);

		printf("%d\n", ways(m,2));

	}

	return 0;

}