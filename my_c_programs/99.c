#include <stdio.h>

/*

 * hehe.cpp

 *

 *  Created on: 2012-11-25

 *      Author:??

 * ???????a?????????????????a = a1 * a2 * a3 * ... * an?

 * ??1 < a1 <= a2 <= a3 <= ... <= an?????????????????a = a???????

 */



int sum = 1;

int Fact(int x, int y);



int main() {

	int n, a;

	scanf("%d", &n);

	for(i=0; i < n; i++) {

		scanf("%d", &a);

		sum = 1;

		Fact(a, 2);

		printf("%d\n", sum);

	}

	return 0;

}

int Fact(int x, int y) {

	if (x == 1)

		return 0;

	else {

		for (; y <= sqrt(x); y++)

			if (x % y == 0) {

				sum = sum + Fact(x / y, y); 

			}

		return 1;

	}



}

