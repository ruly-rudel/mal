#ifndef _builtin_h_
#define _builtin_h_

#include "misc.h"
#include <stdint.h>

typedef enum _rtype_t { CONS_T = 0, NIL_T, SYMBOL_T, INT_T, FLOAT_T, CHAR_T, STR_T, OTHER_T } rtype_t;

typedef struct
{
	uint64_t	main:   3;
	uint64_t	sub:   61;
} type_t;

typedef struct
{
	int64_t		dummy:  3;
	int64_t		val:   61;
} rint_t;

typedef struct
{
	float		dummy;
	float		val;
} rfloat_t;

struct _cons_t;
typedef union
{
	type_t		type;
	rint_t		rint;
	rfloat_t	rfloat;
	struct _cons_t*	cons;
} value_t;

typedef struct _cons_t
{
	value_t		car;
	value_t		cdr;
} cons_t;


#define NIL      ((value_t){ .type.main = NIL_T,  .type.sub = 0 })
#define EOS      ((value_t){ .type.main = STR_T,  .type.sub = 0 })
#define RCHAR(X) ((value_t){ .type.main = CHAR_T, .type.sub = (X) })
#define RINT(X)  ((value_t){ .type.main = INT_T,  .type.sub = (X) })

rtype_t rtypeof(value_t v);
value_t readline	(FILE* fp);
value_t read_str	(value_t s);
void    printline	(value_t s, FILE* fp);
value_t pr_str		(value_t s);
value_t	cons		(value_t car, value_t cdr);

#endif // _builtin_h_
