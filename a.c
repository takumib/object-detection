#include <stdio.h>
#include <stdlib.h>

int main()
{
	float *a;
	int i, j;

	a = malloc(4 * sizeof(float));

	for(i = 0; i < 2; i++)
	{
		for(j = 0; j < 2; j++)
		{
			a[i*2+j] = i * 2 + j;
		}
	}

	for(i = 0; i < 2; i++)
	{
		for(j = 0; j < 2; j++)
		{
			printf("%f ", a[i*2+j]);
		}

		printf("\n");
	}

	printf("%f\n", a[2]);
	return 0;
}
