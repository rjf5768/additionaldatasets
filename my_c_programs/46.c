#include <stdio.h>

int f(int,int);      

int main( )      

{      

	int n,i,a[50];

	scanf("%d", &n);      

	for(i=0;i<n;i++){

		scanf("%d", &a)[i];      

		printf("%d", f)(a[i],2) ;      

	}

	return 0;      

}      

      

int f(int n,int p)      



{

	int sum=0,m;

	if(n<p) return 0;

	else{

		for(m=p;m<=n;m++)

			if(n%m==0){

				if(m<n) sum=sum+f(n/m,m);      

				if(m==n) sum=sum+1;      

			}

			return sum; 

	}

}

