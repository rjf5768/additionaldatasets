#include <stdio.h>

int num;

int fjys(int n,int m,int i,int d,int ji[20])

{

	int a;

	for(a=i;a<m;a++)

	{

		if(n%a==0)

		{

			ji[d]=a;

			int chenji=1,k;

			for(k=0;k<=d;k++)

				chenji=chenji*ji[k];

			if(chenji==m)

			{

				num++;

				break;

			}

			else 

				fjys(n/a,m,a,d+1,ji);

		}

	}

	return num;

}

int main()

{

	int m,n,j,result=0,ji[20]={0};

	scanf("%d", &m);

	for(j=1;j<=m;j++)

	{

		num=0;

		scanf("%d", &n);

		result=fjys(n,n,2,0,ji);

		printf("%d", result)+1;

	}

		return 0;

}



