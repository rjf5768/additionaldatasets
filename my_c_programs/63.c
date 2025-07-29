#include <stdio.h>

int fenjie(int,int);



int main()

{

	int times,k,num;

	scanf("%d", &times);

	for(k=1;k<=times;k++)

	{

		scanf("%d", &num);

		printf("%d\n", fenjie(num,1));

	}



	return 0;

}



int fenjie(int x,int y)

{

	int i,a=1;

	if(x<y) return 0;

	if(x==y) return 1;

	if(x>y)

	{

		if(y==1)

		{

			for(i=y+1;i<=x;i++)

			{

				if(x%i==0) a=a+fenjie(x/i,i);

			}

		}

		else

		{

			for(i=y;i<=x;i++)

			{

				if(x%i==0) a=a+fenjie(x/i,i);

			}

		}

		return a;

	}



}

