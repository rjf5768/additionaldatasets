#include <stdio.h>

void factor(int,int);

int result[9999]={0};

int j=0;

int main()

{

	int times,k=1;

	scanf("%d", &times);

	while(k<=times){

	int num,ansf=0;

	scanf("%d", &num);

	factor(2,num);



	for(i=0;i<9999;i++)

		if(result[i])ansf++;

	printf("%d\n", ansf);



	for(i=0;i<9999;i++)

		result[i]=0;



	k++;

	}





	return 0;

}



void factor(int start,int num)

{  

  if(num==1)

     {result[j]=1;j++;}

   

   for(i=start;i<=num;i++){

	   if(num%i==0)

		   factor(i,num/i);

     }

}



