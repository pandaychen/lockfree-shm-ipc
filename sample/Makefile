
CC = g++
CFLAGS = -Wall -Wno-unknown-pragmas -ggdb -Wno-format
LSI_LIB = ../liblsi.a
LSI_INC = ../

.PHONY: all clean

all: test

test: test.o
	${CC} -o $@ $(CFLAGS) $< ${LSI_LIB}

test.o: test.c
	${CC} ${CFLAGS} -I${LSI_INC} -c -o $@ $<
	
clean:
	@(rm -f *.o test)
