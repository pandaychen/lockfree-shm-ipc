#  @author:   gaccob
#  @date:     2012-12-3
#  @function: lsi lib makefile

TARGET_LIB = liblsi.a 
TARGET_BIN = lsi
CC = g++
AR = ar rc
CFLAGS = -Wall -Wno-unknown-pragmas -ggdb -Wno-format
XML_LIB = xml/libtinyxml.a
XML_INC = xml/

SRCS := hash.c lsi.c lsi_ctl.c lsi_tool.c
OBJS := hash.o lsi.o lsi_ctl.o lsi_tool.o

.PHONY: all clean

all: $(TARGET_LIB) $(TARGET_BIN)

${TARGET_LIB}: hash.o lsi.o
	${AR} $@ hash.o lsi.o
	@(echo "make LSI lib complete")

${TARGET_BIN}: lsi_ctl.o lsi_tool.o lsi.o hash.o
	$(CC) -o $@ $(CFLAGS) $^ ${XML_LIB}
	@(echo "make LSI bin complete")

%.o: %.c
	${CC} -c ${CFLAGS} -I${XML_INC} $< -o $@

clean:
	@(rm -f core hash.o lsi.o lsi_ctl.o lsi_tool.o ${TARGET_LIB} ${TARGET_BIN})
	
depend:
	hash.o: type.h hash.h
	lsi.o: type.h hash.h lsi_tool.h lsi.h
	lsi_ctl.o: lsi_ctl.h lsi_tool.h
	lsi_tool.o: lsi.h lsi_ctl.h lsi_tool.h
