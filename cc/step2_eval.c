
#define DEF_EXTERN
#include <stdio.h>
#include "builtin.h"
#include "reader.h"
#include "printer.h"
#include "env.h"
#include "eval.h"

value_t read(FILE* fp)
{
	value_t str = readline(fp);
	if(errp(str))
	{
		return str;
	}
	else
	{
		return read_str(str);
	}
}

value_t eval(value_t v)
{
	return init_env();
	//return v;
}

void print(value_t s, FILE* fp)
{
	printline(pr_str(s, NIL), fp);
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
			if(r.type.val == ERR_EOF)
			{
				fprintf(stdout, "\n");
				break;
			} 
		}
		else
		{
			e = eval(r);
			print(e, stdout);
		}
	}

	return 0;
}
