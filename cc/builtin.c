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
	return rtypeof(x) == CONS_T && x.type.sub != 0 ? x.cons->car : NIL;
}

value_t cdr(value_t x)
{
	return rtypeof(x) == CONS_T && x.type.sub != 0 ? x.cons->cdr : NIL;
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

bool errp(value_t x)
{
	return x.type.main == ERR_T;
}

bool nilp(value_t x)
{
	return x.type.main == CONS_T && x.type.sub == 0;
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


static value_t* cons_and_cdr(value_t v, value_t* c)
{
	*c = cons(v, NIL);
	return &c->cons->cdr;
}


value_t readline(FILE* fp)
{
	value_t	 r	= NIL;
	value_t* cur	= &r;

	for(;;)
	{
		int c = fgetc(fp);
		if(c == EOF && nilp(r))	// 
		{
			return RERR(ERR_EOF);
		}
		else if(c == '\n' || c == EOF)
		{
			r.type.main = STR_T;
			return r;
		}
		else
		{
			cur = cons_and_cdr(RCHAR(c), cur);
		}
	}
}

void printline(value_t s, FILE* fp)
{
	assert(rtypeof(s) == STR_T || rtypeof(s) == CONS_T);
	s.type.main = CONS_T;

	for(; !nilp(s); s = cdr(s))
		fputc(car(s).rint.val, fp);

	fputc('\n', fp);
	fflush(fp);

	return ;
}


typedef struct
{
	value_t		cur;
	value_t		token;
} scan_t;

static void scan_to_lf(value_t *s)
{
	assert(s != NULL);
	assert(rtypeof(*s) == CONS_T);

	while(!nilp(*s))
	{
		assert(rtypeof(*s) == CONS_T);

		value_t c = car(*s);	// current char
		assert(rtypeof(c) == CHAR_T);

		if(c.rint.val == '\n')
		{
			*s = cdr(*s);
			return ;
		}
		
		*s = cdr(*s);
	}

	return ;
}

static value_t scan_to_whitespace(value_t *s)
{
	assert(s != NULL);
	assert(rtypeof(*s) == CONS_T);

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
			cur = cons_and_cdr(c, cur);

			// next
			*s = cdr(*s);
		}
	}

	// must be refactored
	r.type.main = STR_T;
	return r;
}

static value_t scan_to_doublequote(value_t *s)
{
	assert(s != NULL);
	assert(rtypeof(*s) == CONS_T);

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
				cur = cons_and_cdr(c, cur);
			}
			break;

		    case 1:	// escape
			cur = cons_and_cdr(c, cur);
			st = 0;
			break;
		}

		// next
		*s = cdr(*s);
	}

	assert(0);
	return NIL;	// error: end of source string before doublequote
}

static value_t scan1(value_t *s)
{
	assert(s != NULL);
	assert(rtypeof(*s) == CONS_T);

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
				*s           = cdr (*s);
				scan_to_lf(s);
			        break;	// return to skip-white-space

			    // string
			    case '"':
				*s           = cdr (*s);
				return scan_to_doublequote(s);

			    // atom
			    default:
				return scan_to_whitespace(s);
			}
		}
	}

	return NIL;	// no form or atom
}

static scan_t scan_init(value_t str)
{
	scan_t r = { 0 };
	r.cur    = str;
	if(r.cur.type.main == STR_T)
	{
		r.cur.type.main = CONS_T;
	}
	r.token  = scan1(&r.cur);

	return r;
}

static value_t scan_next(scan_t *s)
{
	assert(s != NULL);

	s->token = scan1(&s->cur);
	return s->token;
}

static value_t scan_peek(scan_t *s)
{
	assert(s != NULL);

	return s->token;
}

/*
static bool scan_is_end(scan_t *s)
{
	assert(s != NULL);

	return nilp(s->cur);
}
*/

static value_t read_form(scan_t* st);

static value_t read_list(scan_t* st)
{
	assert(st != NULL);

	value_t r = NIL;
	value_t *cur = &r;

	value_t token;

	scan_next(st);	// omit '('
	while(!nilp(token = scan_peek(st)))
	{
		assert(rtypeof(token) == STR_T);
		token.type.main = CONS_T;

		value_t c = car(token);	// first char of token

		assert(rtypeof(c) == CHAR_T);
		if(c.rint.val == ')')
		{
			if(nilp(r))	// (nil. nil)
			{
				r = cons(NIL, NIL);
			}
			scan_next(st);
			return r;
		}
		else
		{
			cur = cons_and_cdr(read_form(st), cur);
		}
	}

	assert(1);
	return NIL;	// error: end of token before ')'
}

static value_t read_form(scan_t* st)
{
	assert(st != NULL);

	value_t token = scan_peek(st);
	if(nilp(token))	// no token
	{
		return NIL;
	}
	else
	{
		assert(rtypeof(token) == STR_T);

		token.type.main = CONS_T;
		value_t tcar = car(token);

		assert(rtypeof(tcar) == CHAR_T);
		if(tcar.rint.val == '(')
		{
			return read_list(st);
		}
		else
		{
			token = scan_peek(st);
			scan_next(st);
			return token;
		}
	}
}

value_t read_str(value_t s)
{
	scan_t st = scan_init(s);

	return read_form(&st);
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

	if(nilp(x))
	{
		r = cons(RCHAR('n'), cons(RCHAR('i'), cons(RCHAR('l'), NIL)));
	}
	else
	{
		do
		{
			if(nilp(r))
			{
				r = cons(RCHAR('('), NIL);
			}
			else
			{
				nconc(r, cons(RCHAR(' '), NIL));
			}

			nconc(r, pr_str(car(x)));

			x = cdr(x);
		} while(!nilp(car(x)));

		nconc(r, cons(RCHAR(')'), NIL));
	}

	r.type.main = STR_T;

	return r;
}

value_t pr_str(value_t s)
{
	switch(rtypeof(s))
	{
	    case CONS_T:
		return pr_str_cons(s);

	    case SYM_T:
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

