if HAVE_TESTS

if HAVE_SELENG
SELENG_TESTS = test5
endif

CORE_TESTS=test0 test1 test2 test3 test4 test6

PRGS = $(CORE_TESTS) $(SELENG_TESTS)

noinst_PROGRAMS=$(PRGS)
test0_SOURCES=test0-main.c
test1_SOURCES=test1-main.c 
test2_SOURCES=test2-main.c cr-test-utils.c cr-test-utils.h
test3_SOURCES=test3-main.c cr-test-utils.c cr-test-utils.h	
test4_SOURCES=test4-main.c cr-test-utils.c cr-test-utils.h
test5_SOURCES=test5-main.c cr-test-utils.c cr-test-utils.h
test6_SOURCES=test6-main.c cr-test-utils.c cr-test-utils.h



LDADD=$(top_srcdir)/src/libcroco.la

INCLUDES=-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src \
	`pkg-config --cflags glib-2.0`

LDFLAGS=`pkg-config --libs glib-2.0` @LIBXML2_LIBS@
AM_CFLAGS=-Wall -I. @LIBXML2_CFLAGS@

endif