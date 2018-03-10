#include <assert.h>
#include "builtin.h"
#include "env.h"
#include "util.h"


/////////////////////////////////////////////////////////////////////
// private: support functions searching environment with symbol-string

static value_t add(value_t body, value_t env)
{
	assert(rtypeof(body) == CONS_T);

	value_t sum = RINT(0);
	if(!nilp(body))
	{
		for(value_t cur = body; !nilp(cur); cur = cdr(cur))
		{
			value_t cura = car(cur);
			sum = RINT(sum.rint.val + cura.rint.val);
		}

	}
	return sum;
}

static value_t sub(value_t body, value_t env)
{
	assert(rtypeof(body) == CONS_T);
	if(nilp(body))
	{
		return RERR(ERR_ARG);
	}
	else
	{
		value_t sum = car(body);
		assert(rtypeof(sum) == INT_T);
		body = cdr(body);
		if(!nilp(body))
		{
			for(value_t cur = body; !nilp(cur); cur = cdr(cur))
			{
				value_t cura = car(cur);
				sum = RINT(sum.rint.val - cura.rint.val);
			}
		}

		return sum;
	}
}

static value_t mul(value_t body, value_t env)
{
	assert(rtypeof(body) == CONS_T);

	value_t sum = RINT(1);
	if(!nilp(body))
	{
		for(value_t cur = body; !nilp(cur); cur = cdr(cur))
		{
			value_t cura = car(cur);
			sum = RINT(sum.rint.val * cura.rint.val);
		}

	}
	return sum;
}

static value_t div(value_t body, value_t env)
{
	assert(rtypeof(body) == CONS_T);
	if(nilp(body))
	{
		return RERR(ERR_ARG);
	}
	else
	{
		value_t sum = car(body);
		assert(rtypeof(sum) == INT_T);
		body = cdr(body);
		if(!nilp(body))
		{
			for(value_t cur = body; !nilp(cur); cur = cdr(cur))
			{
				value_t cura = car(cur);
				sum = RINT(sum.rint.val / cura.rint.val);
			}
		}

		return sum;
	}
}



/////////////////////////////////////////////////////////////////////
// public: Environment create, search and modify functions

value_t	init_env	(void)
{
	value_t env = cons(NIL, NIL);
	set_env(str_to_sym("+"), cfn(RFN(add), env), env);
	set_env(str_to_sym("-"), cfn(RFN(sub), env), env);
	set_env(str_to_sym("*"), cfn(RFN(mul), env), env);
	set_env(str_to_sym("/"), cfn(RFN(div), env), env);

	return env;
}

// search only current environment, set only on current environment
value_t	set_env		(value_t key, value_t val, value_t env)
{
	assert(rtypeof(env) == CONS_T);
	assert(rtypeof(key) == SYM_T);

	value_t r = find_env(key, env);
	if(nilp(r))
	{
		r = rplaca(env, acons(key, val, car(env)));
	}
	else
	{
		rplacd(r, val);
	}

	return r;
}

// search only current environment
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
		return assoc(key, car(env));
	}
}

// search whole environment
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
