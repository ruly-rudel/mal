
CC=gcc-5

LD=$(CC)
AR=ar

DEBUG=-g3
OPT=-O0
CFLAGS=$(OPT) -Wall $(DEBUG) $(INCPATHS) -std=c11
LDFLAGS=$(OPT) $(DEBUG) $(LIBPATHS) -L.

LIBSOURCES=builtin.c scanner.c reader.c printer.c eval.c env.c util.c
LIBOBJS=$(LIBSOURCES:%.c=%.o)

MAINS=$(wildcard step*.c)
TARGETS=$(MAINS:%.c=%)

.PHONY:	all clean

.SUFFIXES: .c .o

all: $(TARGETS)

dist: mal

mal: stepA_mal
	cp $< $@

.deps: *.c *.h
	$(CC) $(CFLAGS) -MM *.c > .deps

$(TARGETS): %: %.o libmal.a
	$(LD) $^ -o $@ $(LDFLAGS)

libmal.a: $(LIBOBJS)
	$(AR) rcs $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(TARGETS) libmal.a .deps mal

-include .deps


### Stats

.PHONY: stats stats-lisp

stats: $(LIBSOURCES) stepA_mal.c
	@wc $^
	@printf "%5s %5s %5s %s\n" `grep -E "^[[:space:]]*//|^[[:space:]]*$$" $^ | wc` "[comments/blanks]"

stats-lisp: Core.cc Environment.cc stepA_mal.c
	@wc $^
	@printf "%5s %5s %5s %s\n" `grep -E "^[[:space:]]*//|^[[:space:]]*$$" $^ | wc` "[comments/blanks]"
