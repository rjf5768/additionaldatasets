#include <stdio.h>

int fenj(int x, int y) {

  int a=(x==1),i;

  for (i=y;i<=x;i++)

	  if(x%i==0)a=a+fenj(x/i,i);

  return a;

}

int main(){

	int k;

	scanf("%d", &k);

	int a[k];

	for(i=0;i<k;i++)scanf("%d", &a)[i];

	for(i=0;i<k;i++)printf("%d\n", fenj(a[i],2));

	return 0;

}