#include <assert.h>
#include "builtin.h"
#include "env.h"
#include "util.h"
#include "printer.h"


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

static value_t prn(value_t body, value_t env)
{
	printline(pr_str(car(body), cons(RINT(0), NIL)), stdout);
	return NIL;
}

static value_t b_list1(value_t body, value_t env)
{
	if(nilp(body))
	{
		return NIL;
	}
	else
	{
		return cons(car(body), b_list1(cdr(body), env));
	}
}


static value_t b_list(value_t body, value_t env)
{
#ifdef MAL
	if(nilp(body))
	{
		return cons(NIL, NIL);
	}
	else
	{
		return cons(car(body), b_list1(cdr(body), env));
	}
#else  // MAL
	return b_list1(body, env);
#endif // MAL
}

static value_t is_list(value_t body, value_t env)
{
	value_t arg = car(body);
#ifdef MAL
	return rtypeof(arg) == CONS_T && !nilp(arg) ? SYM_TRUE : SYM_FALSE;
#else  // MAL
	return rtypeof(arg) == CONS_T ? SYM_TRUE : SYM_FALSE;
#endif // MAL
}

static value_t is_empty(value_t body, value_t env)
{
#ifdef MAL
	value_t arg = car(body);
	return rtypeof(arg) == CONS_T && nilp(car(arg)) && nilp(cdr(arg)) ? SYM_TRUE : SYM_FALSE;
#else  // MAL
	return nilp(car(body)) ? SYM_TRUE : SYM_FALSE;
#endif // MAL
}

static value_t count1(value_t body)
{
	if(nilp(body))
	{
		return RINT(0);
	}
	else if(rtypeof(body) != CONS_T)
	{
		return RERR(ERR_TYPE);
	}
	else
	{
		value_t val = count1(cdr(body));
		if(errp(val))
		{
			return val;
		}
		else
		{
			return RINT(val.rint.val + 1);
		}
	}
}

static value_t count(value_t body, value_t env)
{
	value_t arg = car(body);
	if(rtypeof(arg) != CONS_T)
	{
		return RERR(ERR_TYPE);
	}

#ifdef MAL
	if(nilp(car(arg)))
	{
		return RINT(0);
	}
#endif // MAL
	return count1(arg);
}

static value_t b_equal(value_t body, value_t env)
{
	return equal(car(body), car(cdr(body))) ? SYM_TRUE : SYM_FALSE;
}

static value_t comp(value_t body, bool (*comp_fn)(int64_t, int64_t))
{
	value_t a = car(body);
	value_t b = car(cdr(body));
	if(rtypeof(a) == INT_T && rtypeof(b) == INT_T)
	{
		return comp_fn(a.rint.val, b.rint.val) ? SYM_TRUE : SYM_FALSE;
	}
	else
	{
		return RERR(ERR_TYPE);
	}
}

static bool comp_lt(int64_t a, int64_t b)
{
	return a < b;
}

static bool comp_elt(int64_t a, int64_t b)
{
	return a <= b;
}

static bool comp_mt(int64_t a, int64_t b)
{
	return a > b;
}

static bool comp_emt(int64_t a, int64_t b)
{
	return a >= b;
}

static value_t lt(value_t body, value_t env)
{
	return comp(body, comp_lt);
}

static value_t elt(value_t body, value_t env)
{
	return comp(body, comp_elt);
}

static value_t mt(value_t body, value_t env)
{
	return comp(body, comp_mt);
}

static value_t emt(value_t body, value_t env)
{
	return comp(body, comp_emt);
}



/////////////////////////////////////////////////////////////////////
// public: Environment create, search and modify functions

value_t	create_root_env	(void)
{
	value_t key = list(17,
	                      str_to_sym("nil"),
	                      str_to_sym("true"),
	                      str_to_sym("false"),
	                      str_to_sym("+"),
	                      str_to_sym("-"),
	                      str_to_sym("*"),
	                      str_to_sym("/"),
	                      str_to_sym("prn"),
	                      str_to_sym("list"),
	                      str_to_sym("list?"),
	                      str_to_sym("empty?"),
	                      str_to_sym("count"),
	                      str_to_sym("="),
	                      str_to_sym("<"),
	                      str_to_sym("<="),
	                      str_to_sym(">"),
	                      str_to_sym(">=")
	                  );

	value_t val = list(17,
			      NIL,
			      str_to_sym("true"),
			      str_to_sym("false"),
	                      cfn(RFN(add), NIL),
	                      cfn(RFN(sub), NIL),
	                      cfn(RFN(mul), NIL),
	                      cfn(RFN(div), NIL),
	                      cfn(RFN(prn), NIL),
	                      cfn(RFN(b_list), NIL),
	                      cfn(RFN(is_list), NIL),
	                      cfn(RFN(is_empty), NIL),
	                      cfn(RFN(count), NIL),
	                      cfn(RFN(b_equal), NIL),
	                      cfn(RFN(lt), NIL),
	                      cfn(RFN(elt), NIL),
	                      cfn(RFN(mt), NIL),
	                      cfn(RFN(emt), NIL)
			  );

	return create_env(key, val, NIL);
	/*
	value_t env = cons(NIL, NIL);

	set_env(str_to_sym("+"), cfn(RFN(add), env), env);
	set_env(str_to_sym("-"), cfn(RFN(sub), env), env);
	set_env(str_to_sym("*"), cfn(RFN(mul), env), env);
	set_env(str_to_sym("/"), cfn(RFN(div), env), env);

	return env;
	*/

}

value_t	create_env	(value_t key, value_t val, value_t outer)
{
	assert(rtypeof(outer) == CONS_T);
	assert(rtypeof(key)   == CONS_T);
	assert(rtypeof(val)   == CONS_T);

	return cons(pairlis(key, val), outer);
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
