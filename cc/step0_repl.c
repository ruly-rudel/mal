
#define DEF_EXTERN
#include <stdio.h>
#include "builtin.h"

value_t read(FILE* fp)
{
	return readline(fp);
}

value_t eval(value_t v)
{
	return v;
}

void print(value_t s, FILE* fp)
{
	printline(s, fp);
	return;
}

int main(int argc, char* argv[])
{
	value_t r, e;

	for(;;)
	{
		fprintf(stdout, "mal-user> ");
		r = read(stdin);
		if(errp(r))
		{
			fprintf(stdout, "\n");
			break;
		}
		e = eval(r);
		print(e, stdout);
	}

	return 0;
}
