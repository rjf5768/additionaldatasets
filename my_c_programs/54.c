#include <stdio.h>

int f(int m,int k);



int main()

{

	int n,i,k,m;

	scanf("%d", &n);

	for(i=1;i<=n;i++)

	{

		scanf("%d", &m);

		k=2;

		printf("%d\n", f(m,k));

	}

	

	

	

	

	

	

	return 0;

}

int f(int m,int k)

{

	int m1,sum=1;

	m1=m;

	while(k<=(int)sqrt(m))

	{

		if(m%k==0)

		{

			m=m/k;

			

			sum=sum +f(m,k);

		}

		m=m1;

		k++;

	}

	return sum;

}

	

