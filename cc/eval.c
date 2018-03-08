#include <assert.h>
#include "builtin.h"
#include "eval.h"


/////////////////////////////////////////////////////////////////////
// public: eval

value_t eval_ast	(value_t ast, value_t env)
{
	switch(rtypeof(ast))
	{
	    case SYM_T:
		return ast;

	    case CONS_T:
		return ast;

	    default:
		return ast;
	}
}

// End of File
/////////////////////////////////////////////////////////////////////
