# Basic makefile to compile ggen

LIBBOOSTDIR=/usr/lib/



CC=g++
CFLAGS= -O2 -g
LDFLAGS=  -L $(LIBBOOSTDIR) -lboost_program_options -lboost_regex

.PHONY: all clean

all: ggen

ggen: ggen.o dynamic_properties.o
	$(CC) $^ $(LDFLAGS) -o $@

%.o: %.cpp %.hpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o ggen

