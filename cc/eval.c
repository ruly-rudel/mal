#include <assert.h>
#include "builtin.h"
#include "eval.h"
#include "env.h"

/////////////////////////////////////////////////////////////////////
// private: eval functions

value_t eval_sym(value_t sym, value_t env)
{
	assert(rtypeof(sym) == SYM_T);
	return get_env_value(sym, env);
}

/////////////////////////////////////////////////////////////////////
// public: eval

value_t eval_ast	(value_t ast, value_t env)
{
	switch(rtypeof(ast))
	{
	    case SYM_T:
		return eval_sym(ast, env);

	    case CONS_T:
		return ast;

	    default:
		return ast;
	}
}

// End of File
/////////////////////////////////////////////////////////////////////
