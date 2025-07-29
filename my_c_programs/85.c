#include <stdio.h>

int get(int n,int m)

{

	if(n==1)return 1;

	while(m>=2)

	{

		if(n%m==0)break;

		--m;

	}

	if(m==1)return 0;

	return get(n/m,m)+get(n,m-1);

}

int main()

{

	int t;

	scanf("%d", &t);

	for(i=0;i<t;++i)

	{

		int n;

		scanf("%d", &n);

		printf("%d\n", get(n,n));	

	}

	return 0;

}

