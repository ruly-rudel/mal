#include <stdio.h>
#include <stdlib.h>
#include "builtin.h"

cons_t* _read(FILE* fp)
{
	return 0;
}


cons_t*	cons(const value_t *car, const value_t *cdr)
{
	cons_t* c = (cons_t*)malloc(sizeof(cons_t));
	c->car = *car;
	c->cdr = *cdr;

	return c;
}
