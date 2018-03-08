#ifndef _eval_h_
#define _eval_h_

#include "misc.h"
#include "builtin.h"

value_t eval_ast	(value_t ast, value_t env);

#endif // _eval_h_
