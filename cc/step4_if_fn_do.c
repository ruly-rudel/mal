
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

value_t eval_def(value_t vcdr, value_t env)
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

value_t eval_let(value_t vcdr, value_t env)
{
	if(rtypeof(vcdr) != CONS_T)
	{
		return RERR(ERR_ARG);
	}

	// allocate new environment
	value_t let_env = cons(NIL, env);

	// local symbols
	value_t def = car(vcdr);
	if(rtypeof(def) != CONS_T)
	{
		return RERR(ERR_ARG);
	}

	while(!nilp(def))
	{
		// symbol
		value_t sym = car(def);
		def = cdr(def);
		if(rtypeof(sym) != SYM_T)
		{
			return RERR(ERR_ARG);
		}

		// body
		value_t body = eval(car(def), let_env);
		def = cdr(def);
		if(errp(body))
		{
			return body;
		}
		else
		{
			set_env(sym, body, let_env);
		}
	}

	return eval(car(cdr(vcdr)), let_env);
}

value_t eval_fn(value_t vcdr, value_t env)
{
	if(rtypeof(vcdr) != CONS_T)
	{
		return RERR(ERR_ARG);
	}

	// allocate new environment
	value_t let_env = cons(NIL, env);

	// local symbols
	value_t def = car(vcdr);
	if(rtypeof(def) != CONS_T)
	{
		return RERR(ERR_ARG);
	}

	while(!nilp(def))
	{
		// symbol
		value_t sym = car(def);
		def = cdr(def);
		if(rtypeof(sym) != SYM_T)
		{
			return RERR(ERR_ARG);
		}

		// body
		value_t body = eval(car(def), let_env);
		def = cdr(def);
		if(errp(body))
		{
			return body;
		}
		else
		{
			set_env(sym, body, let_env);
		}
	}

	return eval(car(cdr(vcdr)), let_env);
}

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
		return eval_def(vcdr, env);
	}
	else if(equal(vcar, str_to_sym("let*")))	// let*
	{
		return eval_let(vcdr, env);
	}
	else if(equal(vcar, str_to_sym("fn*")))		// fn*
	{
		return eval_fn(vcdr, env);
	}
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
			if(rtypeof(fn) == CFN_T)
			{
				fn.type.main = CONS_T;
				// apply
				return car(fn).rfn(cdr(ev), env);
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
			else
			{
				print(r, stderr);
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
