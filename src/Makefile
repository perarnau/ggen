# Basic makefile to compile ggen

LIBBOOSTDIR=/usr/lib/



CC=g++
CFLAGS= -O2 -g
LDFLAGS=  -L $(LIBBOOSTDIR) -lboost_program_options -lboost_regex

.PHONY: all clean

all: ggen

ggen: ggen.o
	$(CC) $^ $(LDFLAGS) -o $@

ggen.o: ggen.cpp
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f *.o ggen

