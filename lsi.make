#  @author:   gaccob
#  @date:     2012-12-36
#  @function: lsi makefile head

LSI_HOME = $(HOME)/lsi

LSI_BIN = ${LSI_HOME}/bin
LSI_INC = ${LSI_HOME}/include
LSI_LIB = ${LSI_HOME}/lib
LSI_SRC = ${LSI_HOME}/src
LSI_SAMPLE = ${LSI_HOME}/sample
LSI_XML = ${LSI_HOME}/tinyxml
LSI_XML_LIB = ${LSI_XML}/libtinyxml.a

CC = g++
AR = ar rc
CP = cp
CFLAGS = -Wall -Wno-unknown-pragmas -ggdb -Wno-format
