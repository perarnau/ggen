# Basic makefile to compile ggen

LIBBOOSTDIR=/usr/lib/



CC=g++
CFLAGS= -O2 -g
LDFLAGS=  -L $(LIBBOOSTDIR) -lboost_program_options -lboost_regex

.PHONY: all clean

all: ggen

ggen: ggen.o
	g++-4.1 $^ $(LDFLAGS) -o $@

ggen.o: ggen.cpp
	g++-4.1 $(CFLAGS) -c $^

clean:
	rm -f *.o ggen

