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
	int64_t		type:   3;
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


EXTERN value_t NIL
#ifdef DEF_EXTERN
 =
{
	.type.main	= NIL_T,
	.type.sub	= 0
}
#endif
; 


value_t _read(FILE* fp);
void _print(value_t s, FILE* fp);
cons_t*	alloc_cons(void);
value_t	cons(const value_t car, const value_t cdr);



#endif // _builtin_h_
