#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
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

rtype_t rtypeof(value_t v) { return v.type.main; }

value_t readline(FILE* fp)
{
	int c;
	value_t	 r	= EOS;
	value_t* cur	= &r;

	while((c = fgetc(fp)) != EOF) {
		if(c == '\n')
		{
			return r;
		} else {
			*cur           = cons(RCHAR(c), EOS);
			value_t* nxt   = &cur->cons->cdr;

			cur->type.main = STR_T;
			cur            = nxt;
		}
	}

	return NIL;
}

void printline(value_t s, FILE* fp)
{
	while(rtypeof(s) == STR_T && s.type.sub != 0)
	{
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


value_t pr_str_int_rec(uint64_t x, value_t s)
{
	if(x == 0)
	{
		return s;
	}
	else
	{
		value_t r   = cons(RCHAR('0' + (x % 10)), s);
		r.type.main = STR_T;

		return pr_str_int_rec(x / 10, r);
	}
}

value_t pr_str_int(int64_t x)
{
	// sign
	if(x < 0)
	{
		value_t r   = cons(RCHAR('-'), pr_str_int_rec(-x, NIL));
		r.type.main = STR_T;
		return r;
	}
	else
	{
		return pr_str_int_rec(x, NIL);
	}
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

bool nilp(value_t x)
{
	return x.type.main == NIL_T && x.type.sub == 0;
}


value_t nconc(value_t a, value_t b)
{
	assert((rtypeof(a) == CONS_T && rtypeof(b) == CONS_T) ||
	       (rtypeof(a) == STR_T  && rtypeof(b) == STR_T));

	cons_t* i;
	for(i = aligned_addr(a); !nilp(i->cdr); i = aligned_addr(i->cdr))
		;
	i->cdr = b;

	return a;
}

value_t pr_str_cons(value_t x)
{
	assert(rtypeof(x) == CONS_T);

	value_t r    = NIL;

	while(!nilp(car(x)))
	{
		value_t sp   = cons(RCHAR(' '), NIL);
		sp.type.main = STR_T;
		if(nilp(r))
		{
			r = sp;
		} else {
			nconc(r, sp);
		}

		nconc(r, pr_str(car(x)));

		x = cdr(x);
	}

	rplaca(r, RCHAR('('));

	value_t cl   = cons(RCHAR(')'), NIL);
	cl.type.main = STR_T;
	nconc(r, cl);

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

