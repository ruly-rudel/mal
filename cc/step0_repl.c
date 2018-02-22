
#include <stdio.h>
#include "builtin.h"


int main(int argc, char* argv[])
{
	int c;

	fprintf(stdout, "mal-user> ");

	while((c = fgetc(stdin)) != EOF)
	{
		fputc(c, stdout);
		if(c == '\n')
		{
			fprintf(stdout, "mal-user> ");
		}
	}

	return 0;
}
