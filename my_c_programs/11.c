#include <stdio.h>







int ana(int n,int p)

{

	int i;

	int t,s=1;

	t=pow(n,0.5);

	for(i=p;i<=t;i++)

		if(n%i==0)

			s=s+ana(n/i,i);

	return s;

}



int main()

{

	int n;

	int a[100];

	int i;

	scanf("%d", &n);

	for(i=0;i<n;i++)

		scanf("%d", &a)[i];

	for(i=0;i<n;i++)

		printf("%d\n", ana(a[i],2));

	return 0;

}

