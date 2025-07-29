#include <stdio.h>

int f(int x,int y)

{

	int s=0,i;

	for(i=y;i>1;i--)

	{

		if(x%i==0)

			s+=f(x/i,i);

	}

	if(x==1)

		return 1;

	return s;

}

int main()

{

	int n,i,a[50];

	scanf("%d", &n);

	for(i=1;i<=n;i++)

		scanf("%d", &a)[i];

	for(i=1;i<=n;i++)

		printf("%d\n", f(a[i],a[i]));

	return 0;

}