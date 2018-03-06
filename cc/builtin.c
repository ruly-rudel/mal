#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "builtin.h"

#define MAL

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
	return v.type.main == OTH_T ? v.type.sub : v.type.main;
}

value_t car(value_t x)
{
	if(rtypeof(x) == CONS_T)
	{
		return x.type.sub != 0 ? x.cons->car : NIL;
	}
	else
	{
		return RERR(ERR_TYPE);
	}
}

value_t cdr(value_t x)
{
	if(rtypeof(x) == CONS_T)
	{
		return x.type.sub != 0 ? x.cons->cdr : NIL;
	}
	else
	{
		return RERR(ERR_TYPE);
	}
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

value_t	cloj(value_t car, value_t cdr)
{
	value_t	r	= cons(car, cdr);
	r.type.main	= FN_T;

	return r;
}

bool errp(value_t x)
{
	return x.type.main == OTH_T && x.type.sub == ERR_T;
}

bool nilp(value_t x)
{
	return x.type.main == CONS_T && x.type.sub == 0 && x.type.val == 0;
}

value_t rplaca(value_t x, value_t v)
{
	if(rtypeof(x) == CONS_T || rtypeof(x) == STR_T)
	{
		cons_t* c = aligned_addr(x);
		c->car = v;
	}
	else
	{
		x = RERR(ERR_TYPE);
	}

	return x;
}

value_t rplacd(value_t x, value_t v)
{
	if(rtypeof(x) == CONS_T || rtypeof(x) == STR_T)
	{
		cons_t* c = aligned_addr(x);
		c->cdr = v;
	}
	else
	{
		x = RERR(ERR_TYPE);
	}

	return x;
}

value_t last(value_t x)
{
	if(rtypeof(x) == CONS_T || rtypeof(x) == STR_T || rtypeof(x) == SYM_T)
	{
		if(nilp(x))
		{
			return NIL;
		}
		else
		{
			value_t i;
			for(i.cons = aligned_addr(x); !nilp(cdr(i)); i = cdr(i))
				;
	
			return i;
		}
	}
	else
	{
		return RERR(ERR_TYPE);
	}
}

value_t nconc(value_t a, value_t b)
{
	assert(rtypeof(a) == CONS_T || rtypeof(a) == STR_T || rtypeof(a) == SYM_T);


	value_t l = last(a);
	if(nilp(l))
	{
		a = b;
	}
	else
	{
		if(rtypeof(b) == STR_T || rtypeof(b) == SYM_T)
		{
			b.type.main = CONS_T;
		}
		rplacd(l, b);
	}

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
	assert(rtypeof(s) == STR_T || rtypeof(s) == SYM_T || rtypeof(s) == CONS_T);
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
		assert(rtypeof(c) == INT_T);

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

		assert(rtypeof(c) == INT_T);

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
			r.type.main = SYM_T;
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
	r.type.main = SYM_T;
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

		assert(rtypeof(c) == INT_T);

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

	return RERR(ERR_PARSE);	// error: end of source string before doublequote
}

static value_t scan1(value_t *s)
{
	assert(s != NULL);
	assert(rtypeof(*s) == CONS_T);

	while(!nilp(*s))
	{
		assert(rtypeof(*s) == CONS_T);

		value_t c = car(*s);	// current char
		assert(rtypeof(c) == INT_T);

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
				sr.type.main = SYM_T;
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

static value_t parse_int(value_t token)
{
	assert(rtypeof(token) == STR_T || rtypeof(token) == SYM_T);
	token.type.main = CONS_T;

	// sign (if exists)
	value_t tcar = car(token);
	assert(rtypeof(tcar) == INT_T);
	int64_t sign = 1;
	if(tcar.rint.val == '-' || tcar.rint.val == '+')
	{
		if(tcar.rint.val == '-')
		{
			sign = -1;
		}

		// next character
		token = cdr(token);
		assert(rtypeof(token) == CONS_T);

		if(nilp(token))	// not int
		{
			return RERR(ERR_PARSE);
		}

		tcar  = car(token);
		assert(rtypeof(tcar) == INT_T);
	}

	// value
	uint64_t val = 0;
	while(!nilp(token))
	{
		tcar = car(token);
		assert(rtypeof(tcar) == INT_T);

		int cv = tcar.rint.val - '0';
		if(cv >= 0 && cv <= 9)
		{
			val *= 10;
			val += cv;
			token = cdr(token);
			assert(rtypeof(token) == CONS_T);
		}
		else
		{
			return RERR(ERR_PARSE);
		}
	}

	return RINT(sign * val);
}

static value_t read_atom(scan_t* st)
{
	value_t token = scan_peek(st);

	if(rtypeof(token) == SYM_T)
	{
		value_t rint = parse_int(token);
		if(!errp(rint))
		{
			token = rint;
		}
	}

	scan_next(st);
	return token;
}

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
		assert(rtypeof(token) == STR_T || rtypeof(token) == SYM_T || errp(token));

		if(errp(token))
		{
			return token;	// scan error
		}
		else if(rtypeof(token) == SYM_T)
		{
			token.type.main = CONS_T;

			value_t c = car(token);	// first char of token

			assert(rtypeof(c) == INT_T);
			if(c.rint.val == ')')
			{
				if(nilp(r))	// () is nill
				{
					r = NIL;
				}
				scan_next(st);
				return r;
			}
			else
			{
				cur = cons_and_cdr(read_form(st), cur);
			}
		}
		else
		{
			cur = cons_and_cdr(read_form(st), cur);
		}
	}

	return RERR(ERR_PARSE);	// error: end of token before ')'
}

static value_t read_form(scan_t* st)
{
	assert(st != NULL);

	value_t token = scan_peek(st);
	if(nilp(token))	// no token
	{
		return NIL;
	}
	else if(errp(token))	// scan error
	{
		return token;
	}
	else
	{
		assert(rtypeof(token) == STR_T || rtypeof(token) == SYM_T);
		if(rtypeof(token) == SYM_T)
		{
			value_t tcons   = token;
			tcons.type.main = CONS_T;
			assert(!nilp(tcons));

			value_t tcar = car(tcons);
			assert(rtypeof(tcar) == INT_T);

			if(tcar.rint.val == '(')
			{
				return read_list(st);
			}
			else
			{
				return read_atom(st);
			}
		}
		else
		{
			return read_atom(st);
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
	assert(rtypeof(x) == CONS_T);
	value_t r    = NIL;

	if(nilp(x))
	{
#ifdef MAL
		r = str_to_rstr("()");
#else // MAL
		r = str_to_rstr("nil");
#endif // MAL

	}
	else
	{
		do
		{
			if(nilp(r))
			{
				//r = cons(RCHAR('('), NIL);
				r = str_to_rstr("(");
			}
			else
			{
				//nconc(r, cons(RCHAR(' '), NIL));
				nconc(r, str_to_rstr(" "));
			}

			if(rtypeof(x) == CONS_T)
			{
				nconc(r, pr_str(car(x)));
				x = cdr(x);
			}
			else	// dotted
			{
				nconc(r, str_to_rstr(". "));
				nconc(r, pr_str(x));
				x = NIL;
			}
		} while(!nilp(x));

		//nconc(r, cons(RCHAR(')'), NIL));
		nconc(r, str_to_rstr(")"));
	}

	r.type.main = STR_T;

	return r;
}

value_t pr_str_str(value_t s)
{
	assert(rtypeof(s) == STR_T);
	s.type.main = CONS_T;

	value_t r = cons(RCHAR('"'), NIL);
	value_t *cur = &r.cons->cdr;

	while(!nilp(s))
	{
		value_t tcar = car(s);
		assert(rtypeof(tcar) == INT_T);

		if(tcar.rint.val == '"')	// escape
		{
			cur = cons_and_cdr(RCHAR('\\'), cur);
		}
		cur = cons_and_cdr(tcar, cur);

		s = cdr(s);
		assert(rtypeof(s) == CONS_T);
	}

	*cur = cons(RCHAR('"'), NIL);

	r.type.main = STR_T;
	return r;
}

static value_t pr_str_fn(value_t s)
{
	assert(rtypeof(s) == FN_T);
	s.type.main = CONS_T;

	value_t r = str_to_rstr("(#<FUNCTION> . ");
	nconc(r, pr_str(cdr(s)));
	nconc(r, str_to_rstr(")"));

	return r;
}

value_t pr_str(value_t s)
{
	switch(rtypeof(s))
	{
	    case CONS_T:
		return pr_str_cons(s);

	    case SYM_T:
		return s;

	    case INT_T:
		return pr_str_int(s.rint.val);

	    case STR_T:
		return pr_str_str(s);

	    case FN_T:
		return pr_str_fn(s);

	    default:
		return s;
	}
}

bool eq(value_t x, value_t y)
{
	return x.raw == y.raw;
}

bool equal(value_t x, value_t y)
{
	if(eq(x, y))
	{
		return true;
	}
	else if(rtypeof(x) != rtypeof(y))
	{
		return false;
	}
	else if(rtypeof(x) == CONS_T || rtypeof(x) == SYM_T || rtypeof(x) == STR_T)
	{
		x.type.main = CONS_T;
		y.type.main = CONS_T;

		return equal(car(x), car(y)) && equal(cdr(x), cdr(y));
	}
	else
	{
		return false;
	}
}

value_t str_to_cons	(const char* s)
{
	assert(s != NULL);
	value_t    r = NIL;
	value_t* cur = &r;

	int c;
	while((c = *s++) != '\0')
	{
		cur = cons_and_cdr(RCHAR(c), cur);
	}

	return r;
}

value_t str_to_sym	(const char* s)
{
	value_t r = str_to_cons(s);
	r.type.main = SYM_T;

	return r;
}

value_t str_to_rstr	(const char* s)
{
	value_t r = str_to_cons(s);
	r.type.main = STR_T;

	return r;
}

value_t list(int n, ...)
{
	va_list	 arg;
	value_t    r = NIL;
	value_t* cur = &r;

	va_start(arg, n);
	for(int i = 0; i < n; i++)
	{
		cur = cons_and_cdr(va_arg(arg, value_t), cur);
	}

	va_end(arg);

	return r;
}

value_t eval_ast	(value_t ast)
{
	switch(rtypeof(ast))
	{
	    case SYM_T:
		return ast;

	    case CONS_T:
		return ast;

	    default:
		return ast;
	}
}



