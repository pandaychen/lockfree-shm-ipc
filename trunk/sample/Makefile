
include ../lsi.make

.PHONY: all clean

all: test

test: test.o
	${CC} -o $@ $(CFLAGS) $< ${LSI_LIB}/liblsi.a

test.o: test.c
	${CC} ${CFLAGS} -I${LSI_INC} -c -o $@ $<
	
clean:
	@(rm -f *.o test)
