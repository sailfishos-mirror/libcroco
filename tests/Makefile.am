if HAVE_TESTS

core_tests=@CORE_TESTS@
seleng_tests=@SELENG_TESTS@
layeng_tests=@LAYENG_TESTS@
PRGS = $(core_tests) $(seleng_tests) $(layeng_tests)

#the list of all possible tests goes here.
#The real tests that are gonna be compiled are defined
#in configure.in due to automake AM_CONDITIONAL() being bugged
EXTRA_PROGRAMS=test0 test1 test2 test3 test4 test5 test6 test7
noinst_PROGRAMS=$(PRGS)

test0_SOURCES=test0-main.c
test1_SOURCES=test1-main.c 
test2_SOURCES=test2-main.c cr-test-utils.c cr-test-utils.h
test3_SOURCES=test3-main.c cr-test-utils.c cr-test-utils.h
test4_SOURCES=test4-main.c cr-test-utils.c cr-test-utils.h
test5_SOURCES=test5-main.c cr-test-utils.c cr-test-utils.h
test6_SOURCES=test6-main.c cr-test-utils.c cr-test-utils.h
test7_SOURCES=test7-main.c cr-test-utils.c cr-test-utils.h

croco_parser_lib=$(top_builddir)/src/parser/@CROCO_PARSER_LIB@

if HAVE_SELENG
croco_seleng_lib=$(top_builddir)/src/seleng/@CROCO_SELENG_LIB@
endif

if HAVE_LAYENG
croco_layeng_lib=$(top_builddir)/src/layeng/@CROCO_LAYENG_LIB@ 
endif

LDADD=$(croco_parser_lib) $(croco_seleng_lib) $(croco_layeng_lib)

INCLUDES=-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/parser \
	-I$(top_srcdir)/src/seleng \
	-I$(top_srcdir)/src/layeng \
	`pkg-config --cflags glib-2.0`

LDFLAGS=`pkg-config --libs glib-2.0` @LIBXML2_LIBS@
AM_CFLAGS=-Wall -I. @LIBXML2_CFLAGS@
endif