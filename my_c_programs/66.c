#include <stdio.h>

int f(int x,int y){

	if((int)pow(x,0.5)<y)return 1;

	int d=1;

	for(i=y;i<=(int)pow(x,0.5);i++)

		if((x%i)==0)

			d+=f(x/i,i);

	return d;

}

int main(){

    int n,m;

    scanf("%d", &n);

    for(k=1;k<=n;k++){

    	scanf("%d", &m);

    	printf("%d\n", f(m,2));

    }

	return 0;

}