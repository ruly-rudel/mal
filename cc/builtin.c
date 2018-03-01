#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "builtin.h"


rtype_t rtypeof(value_t v) { return v.type.main; }

static inline value_t car(value_t x)
{
	return rtypeof(x) == CONS_T ? x.cons->car : NIL;
}

static inline value_t cdr(value_t x)
{
	return rtypeof(x) == CONS_T ? x.cons->cdr : NIL;
}

static inline cons_t* aligned_addr(value_t v) { return (cons_t*) (((uint64_t)v.cons) & 0xfffffffffffffff8); }
//static inline cons_t* aligned_addr(cons_t* v) { return (cons_t*) (((uint64_t)v     ) & 0xfffffffffffffff8); }

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


bool nilp(value_t x)
{
	return x.type.main == NIL_T && x.type.sub == 0;
}

value_t rplaca(value_t x, value_t v)
{
	assert(rtypeof(x) == CONS_T || rtypeof(x) == STR_T);
	cons_t* c = aligned_addr(x);
	c->car = v;
	
	return x;
}

value_t rplacd(value_t x, value_t v)
{
	assert(rtypeof(x) == CONS_T || rtypeof(x) == STR_T);
	cons_t* c = aligned_addr(x);
	c->cdr = v;
	
	return x;
}


value_t nconc(value_t a, value_t b)
{
	assert(rtypeof(a) == CONS_T || rtypeof(a) == STR_T);
	assert(rtypeof(b) == CONS_T || rtypeof(b) == STR_T);

	value_t i;
	for(i.cons = aligned_addr(a); !nilp(cdr(i)); i = cdr(i))
		;
	b.type.main = CONS_T;
	rplacd(i, b);

	return a;
}





value_t readline(FILE* fp)
{
	int c;
	value_t	 r	= NIL;
	value_t* cur	= &r;

	while((c = fgetc(fp)) != EOF) {
		if(c == '\n')
		{
			r.type.main    = STR_T;
			return r;
		} else {
			*cur = cons(RCHAR(c), NIL);
			cur  = &cur->cons->cdr;
		}
	}

	return NIL;
}

void printline(value_t s, FILE* fp)
{
	assert(rtypeof(s) == STR_T);
	s.type.main = CONS_T;

	for(; !nilp(s); s = cdr(s))
		fputc(car(s).rint.val, fp);

	fputc('\n', fp);
	fflush(fp);

	return ;
}

value_t read_str(value_t s)
{
	return s;
}


value_t pr_str_int_rec(uint64_t x, value_t s)
{
	if(x == 0)
	{
		return s;
	}
	else
	{
		value_t r   = cons(RCHAR('0' + (x % 10)), s);

		return pr_str_int_rec(x / 10, r);
	}
}

value_t pr_str_int(int64_t x)
{
	value_t r;
	if(x < 0)
	{
		r = cons(RCHAR('-'), pr_str_int_rec(-x, NIL));
	}
	else
	{
		r = pr_str_int_rec(x, NIL);
	}

	r.type.main = STR_T;
	return r;
}


value_t pr_str_cons(value_t x)
{
	value_t r    = NIL;

	while(!nilp(car(x)))
	{
		if(nilp(r))
		{
			r = cons(RCHAR('('), NIL);
		} else {
			nconc(r, cons(RCHAR(' '), NIL));
		}

		nconc(r, pr_str(car(x)));

		x = cdr(x);
	}

	nconc(r, cons(RCHAR(')'), NIL));
	r.type.main = STR_T;

	return r;
}

value_t pr_str(value_t s)
{
	switch(rtypeof(s))
	{
	    case CONS_T:
		return pr_str_cons(s);

	    case NIL_T:
		break;

	    case SYMBOL_T:
		break;

	    case INT_T:
		return pr_str_int(s.rint.val);

	    case STR_T:
		return s;

	    default:
		break;
	}

	return s;
}

