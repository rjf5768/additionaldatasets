#include <stdio.h>

/*

 * 1.cpp

 *

 *  Created on: 2010-12-1

 *      Author: hanzhe

 */





int min(int a,int b){   

	if(a>b)return b;

	else return a;

}

int f(int n,int s){ 

	int k=0,i;

	if(n==1)return 1;

	else if(s==1)return 0;

	else if(s>n)return f(n,n);

	else {for(i=2;i<=s;i++)

		if(n%i==0)k=k+f(n/i,min(i,s));

	return k;

	}

}

int main(){ 

	int n,m;

	scanf("%d", &n);

	while(n>0){

		scanf("%d", &m);

		printf("%d\n", f(m,m));  

		n--;

	}

	return 0;

}  

