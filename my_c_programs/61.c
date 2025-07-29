#include <stdio.h>

int f(int x,int y)

{

	if(y==x)

		return 1; 

	if(y>x)

		return 0;

	if(x%y==0)

		return f(x,y+1)+f(x/y,y);

	else

		return f(x,y+1);

}

int main()

{

	int n,x;

	scanf("%d", &n);

	int z,i;

	i=n;

	while(i>0)

	{

		i=i-1;

		scanf("%d", &x);

		z=f(x,2);

		printf("%d\n", z);

	}

	return 0;

}



