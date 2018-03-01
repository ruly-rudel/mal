
#define DEF_EXTERN
#include <stdio.h>
#include "builtin.h"

value_t read(FILE* fp)
{
	return read_str(readline(fp));
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

	do
	{
		fprintf(stdout, "mal-user> ");
		r = read(stdin);
		e = eval(r);
		print(e, stdout);
	} while ( rtypeof(r) != NIL_T );

	return 0;
}
