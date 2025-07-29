#include <stdio.h>

int number=0;

int main()

{

	void f(int first,int m);

	int n,i,num;

	scanf("%d", &n);

	for(i=0;i<n;i++)

	{

		scanf("%d", &num);

		f(2,num);

		printf("%d", number)/2;

		number=0;

	}

	return 0;

}

void f(int first,int m)

{

	int i;

	for(i=first;i<=m;i++)

	{

		

		if(m%i==0)

		{

			m=m/i; 

			f(i,m);	

			if(m!=1)

			m=m*i;

		

		}

	}

	if(m==1)

		number+=1;  

}

