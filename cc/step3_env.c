
#define DEF_EXTERN
#include <stdio.h>
#include <assert.h>
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

value_t eval(value_t v, value_t env);

value_t eval_list(value_t v, value_t env)
{
	assert(rtypeof(v) == CONS_T);

	// some special evaluations
	value_t vcar = car(v);
	value_t vcdr = cdr(v);

	if(rtypeof(vcar) != SYM_T)
	{
		return RERR(ERR_NOTFN);
	}
	else if(equal(vcar, str_to_sym("def!")))	// def!
	{
		// key
		value_t key = car(vcdr);
		if(rtypeof(key) != SYM_T)
		{
			return RERR(ERR_NOTSYM);
		}

		// value
		value_t val_notev = cdr(vcdr);
		if(rtypeof(val_notev) != CONS_T)
		{
			return RERR(ERR_ARG);
		}

		value_t val = eval(car(val_notev), env);

		if(!errp(val))
		{
			set_env(key, val, env);
		}
		return val;
	}
//	else if(equal(vcar, str_to_sym("let*")))	// let*
//	{
//	}
	else						// apply function
	{

		value_t ev = eval_ast(v, env);
		if(errp(ev))
		{
			return ev;
		}
		else
		{
			value_t fn = car(ev);
			if(rtypeof(fn) == FN_T)
			{
				fn.type.main = CONS_T;
				// apply
				return car(fn).rfn(cdr(ev));
			}
			else
			{
				return RERR(ERR_NOTFN);
			}
		}
	}


}

value_t eval(value_t v, value_t env)
{
	if(errp(v))
	{
		return v;
	}
	else if(rtypeof(v) == CONS_T)
	{
		if(nilp(v))
		{
			return NIL;
		}
		else
		{
			return eval_list(v, env);
		}
	}
	else
	{
		return eval_ast(v, env);
	}
}

void print(value_t s, FILE* fp)
{
	printline(pr_str(s, cons(RINT(0), NIL)), fp);
	return;
}

int main(int argc, char* argv[])
{
	value_t r, e;
	value_t env = init_env();

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
			e = eval(r, env);
			if(!errp(e))
			{
				print(e, stdout);
			}
			else
			{
				print(e, stderr);
			}
		}
	}

	return 0;
}
