#include <stdio.h>



int s(int a,int b)

{  int sum=0,z=0;

	if(a!=1&&b<=a)

	{

	  for(i=b;i<=a;++i)

   {

	   if(a%i==0)

	   {

		   sum+=s(a/i,i);z=1;

	   }

   }

	 

	}

	else if(a==1)

		return 1;



	return sum;



}

int main()

{  

	int n,a;

	scanf("%d", &n);

	for(i=0;i<n;++i)

	{   

		scanf("%d", &a);

		printf("%d\n", s(a,2));

	}

	return 0;

}



