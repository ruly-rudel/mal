
#define DEF_EXTERN
#include <stdio.h>
#include "builtin.h"


int main(int argc, char* argv[])
{
	value_t s;

	for(;;)
	{
		fprintf(stdout, "mal-user> ");
		s = _read(stdin);
		if(s.type.main == NIL_T) break;
		_print(s, stdout);
	}

	return 0;
}
