#include <stdio.h>





int ans;



void dfs(int, int);



int main()

{

	int n, num;

	scanf("%d", &n);

	while (n--) {

		scanf("%d", &num);

		ans = 0;

		dfs(num, 2);

		printf("%d\n", ans);		

	}

	return 0;

}



void dfs(int x, int f)

{

	if (x == 1) {

		++ans;

		return ;

	}

	for(i=f; i <= x; ++i) 

		if (x % i == 0) 

			dfs(x / i, i);

	return ;

}