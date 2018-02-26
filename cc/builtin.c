#include <stdio.h>
#include <stdlib.h>
#include "builtin.h"


static inline value_t car(value_t x)
{
	return rtypeof(x) == CONS_T ? x.cons->car : NIL;
}

static inline value_t cdr(value_t x)
{
	return rtypeof(x) == CONS_T ? x.cons->cdr : NIL;
}

static inline cons_t* aligned_addr(value_t v) { return (cons_t*) (((uint64_t)v.cons) & 0xfffffffffffffff8); }

static cons_t*	alloc_cons(void)
{
	cons_t* c = (cons_t*)malloc(sizeof(cons_t));

	return c;
}

value_t	cons(value_t car, value_t cdr)
{
	value_t	r	= { 0 };
	// r.type.main is always 0 (=CONS_T) because malloc returns ptr aligned to 8byte.
	// r.type.main	= CONS_T;
	r.cons		= alloc_cons();
	r.cons->car	= car;
	r.cons->cdr	= cdr;

	return r;
}

rtype_t rtypeof(value_t v) { return v.type.main; }

value_t readline(FILE* fp)
{
	int c;
	cons_t	 r	= { 0 };
	value_t* cur	= &r.cdr;


	while((c = fgetc(fp)) != EOF) {
		if(c == '\n')
		{
			// null terminate
			cur->type.main    = STR_T;
			cur->type.sub     = 0;

			return r.cdr;
		} else {
			cons_t* nc        = alloc_cons();
			nc->car.type.main = CHAR_T;
			nc->car.rint.val  = c;

			cur->cons         = nc;
			cur->type.main    = STR_T;

			cur               = &nc->cdr;
		}
	}

	return NIL;
}

void printline(value_t s, FILE* fp)
{
	while(rtypeof(s) == STR_T && s.type.sub != 0) {
		cons_t* c = aligned_addr(s);
		fputc(c->car.rint.val, fp);
		s = c->cdr;
	}

	fputc('\n', fp);
	fflush(fp);

	return ;
}

value_t read_str(value_t s)
{
	return s;
}

value_t pr_str(value_t s)
{
	return s;
}

