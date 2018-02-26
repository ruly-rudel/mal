#ifndef _builtin_h_
#define _builtin_h_

#include <stdint.h>

typedef enum _rtype_t { INT_T = 0, CHAR_T, SYMBOL_T, CONS_T } rtype_t;

struct _cons_t;
typedef union
{
	struct _cons_t*	cons;
	int		integer;
	char		character;
} content_t;

typedef struct
{
	rtype_t		type;
	content_t	content;
} value_t;

typedef struct _cons_t
{
	value_t		car;
	value_t		cdr;
} cons_t;

cons_t* _read(FILE* fp);
cons_t*	cons(const value_t *car, const value_t *cdr);


#endif // _builtin_h_
