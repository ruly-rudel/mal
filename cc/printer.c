#include <assert.h>
#include "builtin.h"
#include "printer.h"
#include "util.h"

#define MAL

/////////////////////////////////////////////////////////////////////
// private: printer support functions

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
	else if(x == 0)
	{
		r = str_to_rstr("0");
	}
	else
	{
		r = pr_str_int_rec(x, NIL);
	}

	r.type.main = STR_T;
	return r;
}


static value_t pr_str_cons(value_t x, value_t cyclic)
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
				r = str_to_rstr("(");
			}
			else
			{
				nconc(r, str_to_rstr(" "));
			}

			if(rtypeof(x) == CONS_T)
			{
				// check cyclic
				if(!nilp(cyclic))
				{
					value_t ex = search_alist(cdr(cyclic), x);
					if(nilp(ex))
					{
						nconc(cyclic, cons(cons(x, NIL), NIL));
						// append string
						nconc(r, pr_str(car(x), cyclic));
					}
					else
					{
						value_t n = cdr(ex);
						if(nilp(n))
						{
							n = car(cyclic);
							assert(intp(n));
							rplacd(ex, n);
							rplaca(cyclic, RINT(n.rint.val + 1));
						}

						nconc(r, str_to_rstr("#"));
						nconc(r, pr_str_int(n.rint.val));
						nconc(r, str_to_rstr("#"));
					}
				}
				else	// does not chech cyclic
				{
					// append string
					nconc(r, pr_str(car(x), cyclic));
				}

				x = cdr(x);
			}
			else	// dotted
			{
				nconc(r, str_to_rstr(". "));
				nconc(r, pr_str(x, cyclic));
				x = NIL;
			}
		} while(!nilp(x));

		nconc(r, str_to_rstr(")"));
	}

	r.type.main = STR_T;

	return r;
}

static value_t pr_str_str(value_t s)
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

static value_t pr_str_fn(value_t s, value_t cyclic)
{
	assert(rtypeof(s) == FN_T);
	s.type.main = CONS_T;

	value_t r = str_to_rstr("(#<FUNCTION> . ");
	nconc(r, pr_str(cdr(s), cyclic));
	nconc(r, str_to_rstr(")"));

	return r;
}



/////////////////////////////////////////////////////////////////////
// public: Rudel-specific functions

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

value_t pr_str(value_t s, value_t cyclic)
{
	switch(rtypeof(s))
	{
	    case CONS_T:
		return pr_str_cons(s, cyclic);

	    case SYM_T:
		return s;

	    case INT_T:
		return pr_str_int(s.rint.val);

	    case STR_T:
		return pr_str_str(s);

	    case FN_T:
		return pr_str_fn(s, cyclic);

	    default:
		return s;
	}
}

// End of File
/////////////////////////////////////////////////////////////////////
