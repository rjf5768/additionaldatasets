#include <stdio.h>

int m=2;

int manners(int a){

	int count=1,t=0;

	t=sqrt((double) a);

	for(i=m;i<=t;i++){

		if(a%i==0){

			m=i;

			count=count+manners(a/i);

		}

	}

	return count;

}

int main(){

	int n, b;

	int i=1;

	scanf("%d", &n);

	while(i<=n){

		scanf("%d", &b);

		printf("%d\n", manners(b));

		m=2;

		i++;

	}

   return 0;

}