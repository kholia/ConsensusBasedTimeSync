#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	double dt = 0;
	char *line = "0    17  -0.5 2350 ~  VU3CER VU3FOE MK6";
	char *eptr;
	double result;
	char *l = strdup(line);
	char *token = strtok(l, " ");

	token = strtok(NULL, " ");
	token = strtok(NULL, " ");
	puts(token);
	result = strtod(token, &eptr);
	printf("%lf\n", result);

	free(l);

	return 0;
}
