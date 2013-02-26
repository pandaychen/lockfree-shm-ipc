#  @author:   gaccob
#  @date:     2012-12-3
#  @function: lsi lib & tool makefile

include ../lsi.make

TARGET_LIB = liblsi.a 
TARGET_BIN = lsi

SRCS := hash.c lsi.c lsi_ctl.c lsi_tool.c xml_wrapper.c
OBJS := hash.o lsi.o lsi_ctl.o lsi_tool.o xml_wrapper.o

.PHONY: all clean

all: $(TARGET_LIB) $(TARGET_BIN)

${TARGET_LIB}: hash.o lsi.o xml_wrapper.o
	${AR} $@ hash.o lsi.o xml_wrapper.o
	${CP} $@ ${LSI_LIB}
	@(echo "make LSI lib complete")

${TARGET_BIN}: lsi_ctl.o lsi_tool.o lsi.o hash.o xml_wrapper.o
	$(CC) -o $@ $(CFLAGS) $^ ${LSI_XML_LIB} ${LSI_THREAD_LIB}
	${CP} $@ ${LSI_BIN}
	@(echo "make LSI bin complete")

%.o: %.c
	${CC} -c ${CFLAGS} -I${LSI_XML_INC} -I${LSI_INC} $< -o $@

clean:
	@(rm -f core hash.o lsi.o lsi_ctl.o lsi_tool.o xml_wrapper.o ${TARGET_LIB} ${TARGET_BIN} ${LSI_BIN}/${TARGET_BIN} ${TARGET_LIB}/${TARGET_LIB})
	
depend:
	hash.o: type.h hash.h
	lsi.o: type.h hash.h lsi_tool.h lsi.h
	lsi_ctl.o: lsi_ctl.h lsi_tool.h
	lsi_tool.o: lsi.h lsi_ctl.h lsi_tool.h
	xml_wrapper.o: xml_wrapper.h