#include <assert.h>
#include "builtin.h"
#include "eval.h"
#include "env.h"

value_t eval(value_t v, value_t env);

/////////////////////////////////////////////////////////////////////
// private: eval functions


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

value_t eval_ast_list(value_t list, value_t env)
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
	value_t let_env = create_env(NIL, NIL, env);

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

// End of File
/////////////////////////////////////////////////////////////////////
