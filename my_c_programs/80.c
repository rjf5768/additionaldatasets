#include <stdio.h>

int su(int input)

{

	int ju=1;

	for(i=2;i<sqrt(input);i++)

	{

		if(input%i==0) ju=0;

		break;

	}

	return ju;

}

int fenjie(int input, int k)

{

	int ret=1;

	for(i=k;i<=sqrt(input);i++)

	{

		if(input%i==0)

		{

			if(su(input/i)==0);

			{

				ret=fenjie(input/i,i)+ret;

			}

		}

	}

	return ret;

}

int main()

{

	int input,num,sol;

	scanf("%d", &num);

	while(num--)

	{

		scanf("%d", &input);

		printf("%d\n", fenjie(input,2));

	}

	return 0;

}

