
#CFLAG       = $(CFLAGS) -D_GNU_SOURCE -Wall -std=gnu99 -g -DASN_EMIT_DEBUG -DASN_DEBUG -DASN_DISABLE_STACK_OVERFLOW_CHECK
CFLAG       = $(CFLAGS) -D_GNU_SOURCE -Wall -std=gnu99 -g -DASN_DEBUG -DASN_DISABLE_STACK_OVERFLOW_CHECK -DASN_EMIT_DEBUG=1 -DJUNKTEST

BUILD_INC_PATH  = -I../build/include -I../build/include/libxml2
BUILD_LIBS_PATH = ../build/lib
BUILD_LIBS = $(BUILD_LIBS_PATH)/libxml2.a \
			 $(BUILD_LIBS_PATH)/libjson-c.a \
			 $(BUILD_LIBS_PATH)/libxml2json2xml.a \
			 $(BUILD_LIBS_PATH)/libasncodec.a \
			 -lm -lz

SRCS        = main.c

OBJS        = $(SRCS:.c=.o)

AOUT        = test

DEST        = ../../../bin

#
#---------------------------------------------------------------------
#
all:: clean $(AOUT) install

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAG) $(BUILD_INC_PATH) $(INC_PATH) -c $<

$(AOUT): $(OBJS)
	$(CC) $(CFLAG) -o $(AOUT) $(OBJS) $(BUILD_LIBS) $(LIB_PATH) $(LIBS) 

install:
#	\cp $(AOUT) $(DEST)

clean:
	\rm -f $(OBJS) $(AOUT) core.* 
	rm -rf cscope.out tceetree.out tceetree.out.png
