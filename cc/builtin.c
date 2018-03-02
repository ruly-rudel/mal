#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "builtin.h"


static cons_t* aligned_addr(value_t v)
{
	return (cons_t*) (((uint64_t)v.cons) & 0xfffffffffffffff8);
}

static cons_t* alloc_cons(void)
{
	cons_t* c = (cons_t*)malloc(sizeof(cons_t));

	return c;
}



rtype_t rtypeof(value_t v)
{
	return v.type.main;
}

value_t car(value_t x)
{
	return rtypeof(x) == CONS_T ? x.cons->car : NIL;
}

value_t cdr(value_t x)
{
	return rtypeof(x) == CONS_T ? x.cons->cdr : NIL;
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

	while((c = fgetc(fp)) != EOF)
	{
		*cur = cons(RCHAR(c), NIL);
		cur  = &cur->cons->cdr;

		if(c == '\n')
		{
			r.type.main    = STR_T;
			return r;
		}
	}

	return r;
}

void printline(value_t s, FILE* fp)
{
	assert(rtypeof(s) == STR_T || rtypeof(s) == NIL_T);
	if(s.type.main == STR_T)
	{
		if(s.type.sub == 0)
		{
			s = NIL;
		} else {
			s.type.main = CONS_T;
		}
	}

	for(; !nilp(s); s = cdr(s))
		fputc(car(s).rint.val, fp);

	fflush(fp);

	return ;
}

static void scan_to_lf(value_t *s)
{
	assert(s != NULL);
	assert(rtypeof(*s) == NIL_T || rtypeof(*s) == CONS_T);

	while(!nilp(*s))
	{
		assert(rtypeof(*s) == CONS_T);

		value_t c = car(*s);	// current char
		assert(rtypeof(c) == CHAR_T);

		if(c.rint.val == '\n')
		{
			return ;
		}
		
		*s = cdr(*s);
	}

	return ;
}

static value_t scan_to_whitespace(value_t *s)
{
	assert(s != NULL);
	assert(rtypeof(*s) == NIL_T || rtypeof(*s) == CONS_T);

	value_t r = NIL;
	value_t *cur = &r;

	while(!nilp(*s))
	{
		assert(rtypeof(*s) == CONS_T);

		value_t c = car(*s);	// current char

		assert(rtypeof(c) == CHAR_T);

		if( c.rint.val == ' '  ||
		    c.rint.val == '\t' ||
		    c.rint.val == '\n' ||
		    c.rint.val == '('  ||
		    c.rint.val == ')'  ||
		    c.rint.val == '['  ||
		    c.rint.val == ']'  ||
		    c.rint.val == '{'  ||
		    c.rint.val == '}'  ||
		    c.rint.val == ';'  ||
		    c.rint.val == '\'' ||
		    c.rint.val == '"'  ||
		    c.rint.val == ','  
		  )
		{
			r.type.main = STR_T;
			return r;
		}
		else
		{
			*cur = cons(c, NIL);
			cur  = &cur->cons->cdr;

			// next
			*s = cdr(*s);
		}
	}

	assert(1);
	return NIL;	// error: end of source string before doublequote
}

static value_t scan_to_doublequote(value_t *s)
{
	assert(s != NULL);
	assert(rtypeof(*s) == NIL_T || rtypeof(*s) == CONS_T);

	value_t r = NIL;
	value_t *cur = &r;

	int st = 0;
	while(!nilp(*s))
	{
		assert(rtypeof(*s) == CONS_T);

		value_t c = car(*s);	// current char

		assert(rtypeof(c) == CHAR_T);

		switch(st)
		{
		    case 0:	// not escape
			if(c.rint.val == '\\')
			{
				st = 1;	// enter escape mode
			}
			else if(c.rint.val == '"')
			{
				*s = cdr(*s);
				r.type.main = STR_T;
				return r;
			}
			else
			{
				*cur = cons(c, NIL);
			}
			break;

		    case 1:	// escape
			*cur = cons(c, NIL);
			st = 0;
			break;
		}

		// next
		*s = cdr(*s);
	}

	assert(1);
	return NIL;	// error: end of source string before doublequote
}

static value_t scan(value_t *s)
{
	assert(s != NULL);
	assert(rtypeof(*s) == NIL_T || rtypeof(*s) == CONS_T);

	while(!nilp(*s))
	{
		assert(rtypeof(*s) == CONS_T);

		value_t c = car(*s);	// current char
		assert(rtypeof(c) == CHAR_T);

		// skip white space
		if( c.rint.val == ' '  ||
		    c.rint.val == '\t' ||
		    c.rint.val == '\n' ||
		    c.rint.val == ','  
		  )
		{
			*s = cdr(*s);
		}
		else
		{
			switch(c.rint.val)
			{
			    // special character
			    case '~':
			    case '[':
			    case ']':
			    case '{':
			    case '}':
			    case '(':
			    case ')':
			    case '\'':
			    case '`':
			    case '^':
			    case '@':
				*s           = cdr (*s);
				value_t sr   = cons(c, NIL);
				sr.type.main = STR_T;
				return sr;

			    // comment
			    case ';':
				scan_to_lf(s);
			        break;	// return to skip-white-space

			    // string
			    case '"':
				return scan_to_doublequote(s);

			    // atom
			    default:
				return scan_to_whitespace(s);
			}
		}
	}

	return NIL;	// no form or atom
}

value_t read_str(value_t s)
{
	value_t p = s;
	p.type.main = CONS_T;

	value_t r = NIL;
	value_t* cur = &r;

	value_t token;

	while(!nilp(token = scan(&p)))
	{
		*cur = cons(token, NIL);
		cur  = &cur->cons->cdr;
	} 

	return r;
}


static value_t pr_str_int_rec(uint64_t x, value_t s)
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

static value_t pr_str_int(int64_t x)
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


static value_t pr_str_cons(value_t x)
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

