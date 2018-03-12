
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

	if(equal(vcar, str_to_sym("def!")))		// def!
	{
		return eval_def(vcdr, env);
	}
	else if(equal(vcar, str_to_sym("let*")))	// let*
	{
		return eval_let(vcdr, env);
	}
	else if(equal(vcar, str_to_sym("do")))		// do
	{
		return eval_do(vcdr, env);
	}
	else if(equal(vcar, str_to_sym("if")))		// if
	{
		return eval_if(vcdr, env);
	}
	else if(equal(vcar, str_to_sym("fn*")))		// fn*
	{
		return cloj(vcdr, env);
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
			else if(rtypeof(fn) == CLOJ_T)
			{
				fn.type.main = CONS_T;

				value_t fn_ast = car(fn);
				if(rtypeof(fn_ast) != CONS_T)
				{
					return RERR(ERR_ARG);
				}

				// formal arguments from clojure
				value_t fargs = car(fn_ast);
				if(rtypeof(fargs) != CONS_T && rtypeof(fargs) != VEC_T)
				{
					return RERR(ERR_ARG);
				}
				fargs.type.main = CONS_T;

				// function body from clojure
				fn_ast = cdr(fn_ast);
				if(rtypeof(fn_ast) != CONS_T)
				{
					return RERR(ERR_ARG);
				}
				value_t body = car(fn_ast);

				// create bindings (is environment)
				value_t fn_env = create_env(fargs, cdr(ev), cdr(fn));

				// apply
				return eval(body, fn_env);
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
#ifdef MAL
		else if(nilp(car(v)) && nilp(cdr(v)))	// empty list
		{
			return v;
		}
#endif // MAL
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
	printline(pr_str(s, cons(RINT(0), NIL), true), fp);
	return;
}

int main(int argc, char* argv[])
{
	value_t r, e;
	value_t env = create_root_env();

	// implement not
	eval(read_str(str_to_rstr("(def! not (fn* (a) (if a false true)))")), env);

	// implement load_file
	eval(read_str(str_to_rstr("(def! load-file (fn* (f) (eval (read-string (str \"(do \" (slurp f) \")\")))))")), env);

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
