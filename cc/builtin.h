#ifndef _builtin_h_
#define _builtin_h_

#include "misc.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum _rtype_t {
	// main 8types (content is address)
	CONS_T = 0,
	SYM_T,
	STR_T,
	FN_T,
	MACRO_T,
	VEC_T,
	HASH_T,
	OTH_T,

	// sub types (content is value)
	INT_T,
	FLOAT_T,
	ERR_T
} rtype_t;

typedef struct
{
	uint64_t	main:   3;
	uint64_t	sub:   29;
	uint64_t	val:   32;
} type_t;

typedef struct
{
	int64_t		type:  32;
	int64_t		val:   32;
} rint_t;

typedef struct
{
	uint32_t	type;
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


#define NIL       ((value_t){ .type.main   = CONS_T,  .type.sub   = 0,     .type.val = 0   })
#define RERR(X)   ((value_t){ .type.main   = OTH_T,   .type.sub   = ERR_T, .type.val = (X) })

#define RCHAR(X)  ((value_t){ .rint.type   = INT_T   << 3 | OTH_T, .rint.val   = (X) })
#define RINT(X)   ((value_t){ .rint.type   = INT_T   << 3 | OTH_T, .rint.val   = (X) })
#define RFLOAT(X) ((value_t){ .rfloat.type = FLOAT_T << 3 | OTH_T, .rfloat.val = (X) })

#define ERR_TYPE	1
#define ERR_EOF		2
#define ERR_PARSE	3

rtype_t rtypeof	(value_t v);

value_t car	(value_t x);
value_t cdr	(value_t x);
value_t	cons	(value_t car, value_t cdr);
bool    errp	(value_t x);
bool    nilp	(value_t x);
value_t rplaca	(value_t x, value_t v);
value_t rplacd	(value_t x, value_t v);
value_t last	(value_t x);
value_t nconc	(value_t a, value_t b);

value_t readline	(FILE* fp);
value_t read_str	(value_t s);
void    printline	(value_t s, FILE* fp);
value_t pr_str		(value_t s);

#endif // _builtin_h_
