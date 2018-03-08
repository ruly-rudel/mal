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
	set_env(env, str_to_sym("+"), cloj(RFN(add), env));
	set_env(env, str_to_sym("-"), cloj(RFN(sub), env));
	set_env(env, str_to_sym("*"), cloj(RFN(mul), env));
	set_env(env, str_to_sym("/"), cloj(RFN(div), env));

	return env;
}

value_t	set_env		(value_t env, value_t key, value_t val)
{
	assert(rtypeof(env) == CONS_T);
	assert(rtypeof(key) == SYM_T);

	value_t r = find_env(env, key);
	if(nilp(r))
	{
		r = cons(key, val);
		rplaca(env, nconc(car(env), list(1, r)));
	}
	else
	{
		rplacd(r, val);
	}

	return r;
}

value_t	find_env	(value_t env, value_t key)
{
	assert(rtypeof(env) == CONS_T);
	assert(rtypeof(key) == SYM_T);

	if(nilp(env))
	{
		return NIL;
	}
	else
	{
		value_t s = search_alist(car(env), key);
		if(nilp(s))
		{
			return find_env(cdr(env), key);
		}
		else
		{
			return s;
		}
	}
}

value_t	get_env_value	(value_t env, value_t key)
{
	assert(rtypeof(env) == CONS_T);
	assert(rtypeof(key) == SYM_T);

	if(nilp(env))
	{
		return RERR(ERR_NOTFOUND);
	}
	else
	{
		value_t s = search_alist(car(env), key);
		if(nilp(s))
		{
			return get_env_value(cdr(env), key);
		}
		else
		{
			return cdr(s);
		}
	}
}

// End of File
/////////////////////////////////////////////////////////////////////
