#include <assert.h>
#include "builtin.h"
#include "env.h"
#include "util.h"


/////////////////////////////////////////////////////////////////////
// private: support functions searching environment with symbol-string

static value_t add(value_t a, value_t b)
{
	return RINT(a.rint.val + b.rint.val);
}

static value_t sub(value_t a, value_t b)
{
	return RINT(a.rint.val - b.rint.val);
}

static value_t mul(value_t a, value_t b)
{
	return RINT(a.rint.val * b.rint.val);
}

static value_t div(value_t a, value_t b)
{
	return RINT(a.rint.val / b.rint.val);
}



/////////////////////////////////////////////////////////////////////
// public: Environment create, search and modify functions

value_t	init_env	(void)
{
	value_t env = cons(NIL, NIL);
	set_env(str_to_sym("+"), cloj(RFN(add), env), env);
	set_env(str_to_sym("-"), cloj(RFN(sub), env), env);
	set_env(str_to_sym("*"), cloj(RFN(mul), env), env);
	set_env(str_to_sym("/"), cloj(RFN(div), env), env);

	return env;
}

value_t	set_env		(value_t key, value_t val, value_t env)
{
	assert(rtypeof(env) == CONS_T);
	assert(rtypeof(key) == SYM_T);

	value_t r = find_env(key, env);
	if(nilp(r))
	{
		rplaca(env, acons(key, val, car(env)));
	}
	else
	{
		rplacd(r, val);
	}

	return r;
}

value_t	find_env	(value_t key, value_t env)
{
	assert(rtypeof(env) == CONS_T);
	assert(rtypeof(key) == SYM_T);

	if(nilp(env))
	{
		return NIL;
	}
	else
	{
		value_t s = assoc(key, car(env));
		if(nilp(s))
		{
			return find_env(key, cdr(env));
		}
		else
		{
			return s;
		}
	}
}

value_t	get_env_value	(value_t key, value_t env)
{
	assert(rtypeof(env) == CONS_T);
	assert(rtypeof(key) == SYM_T);

	if(nilp(env))
	{
		return RERR(ERR_NOTFOUND);
	}
	else
	{
		value_t s = assoc(key, car(env));
		if(nilp(s))
		{
			return get_env_value(key, cdr(env));
		}
		else
		{
			return cdr(s);
		}
	}
}

// End of File
/////////////////////////////////////////////////////////////////////
