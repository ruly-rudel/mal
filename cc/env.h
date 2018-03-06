#ifndef _env_h_
#define _env_h_

#include "misc.h"
#include <stdint.h>
#include <stdbool.h>
#include "builtin.h"

value_t	init_env	(void);
value_t	set_env		(value_t env, value_t key, value_t val);
value_t	find_env	(value_t env, value_t key);
value_t	get_env_value	(value_t env, value_t key);

#endif // _env_h_
