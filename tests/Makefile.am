if HAVE_TESTS 

noinst_PROGRAMS=test0 test1 test2 test3 test4 test5

test0_SOURCES=test0-main.c 
test1_SOURCES=test1-main.c 
test2_SOURCES=test2-main.c cr-test-utils.c cr-test-utils.h
test3_SOURCES=test3-main.c cr-test-utils.c cr-test-utils.h	
test4_SOURCES=test4-main.c cr-test-utils.c cr-test-utils.h
test5_SOURCES=test5-main.c cr-test-utils.c cr-test-utils.h

endif


LDADD=$(top_srcdir)/src/libcroco.la

INCLUDES=-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src \
	`pkg-config --cflags glib-2.0`

LDFLAGS=`pkg-config --libs glib-2.0` @LIBXML2_LIBS@
AM_CFLAGS=-Wall -I. @LIBXML2_CFLAGS@
