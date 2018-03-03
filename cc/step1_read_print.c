
#define DEF_EXTERN
#include <stdio.h>
#include "builtin.h"

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
//	return (value_t){ .type.main = INT_T, .type.sub = -732954 };
/*
	value_t ch = cons(RCHAR('+'), NIL);
	ch.type.main = STR_T;
	return cons(ch, cons(cons(RINT(-243), cons(RINT(9573), NIL)), cons(RINT(321), NIL)));
*/
	return v;
}

void print(value_t s, FILE* fp)
{
	printline(pr_str(s), fp);
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
			if(r.type.sub == ERR_EOF)
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
