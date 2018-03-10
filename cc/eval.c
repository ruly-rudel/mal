#include <assert.h>
#include "builtin.h"
#include "eval.h"
#include "env.h"

value_t eval(value_t v, value_t env);

/////////////////////////////////////////////////////////////////////
// private: eval functions

static value_t eval_ast_list(value_t list, value_t env)
{
	assert(rtypeof(list) == CONS_T);

	if(nilp(list))
	{
		return NIL;
	}
	else
	{
		value_t lcar = eval(car(list), env);
		if(errp(lcar))
		{
			return lcar;
		}

		value_t lcdr = eval_ast_list(cdr(list), env);
		if(errp(lcdr))
		{
			return lcdr;
		}

		return cons(lcar, lcdr);
	}
}

/////////////////////////////////////////////////////////////////////
// public: eval

value_t eval_ast	(value_t ast, value_t env)
{
	switch(rtypeof(ast))
	{
	    case SYM_T:
		return get_env_value(ast, env);

	    case CONS_T:
		return eval_ast_list(ast, env);

	    default:
		return ast;
	}
}

// End of File
/////////////////////////////////////////////////////////////////////
