SUBDIRS=test-inputs test-output-refs

#the list of all possible tests goes here.
#The real tests that are gonna be compiled are defined
#in configure.in due to automake AM_CONDITIONAL() being bugged
testprogs=test0 test1 test2 test3 test4 test5 test6
noinst_PROGRAMS=$(testprogs)
test0_SOURCES=test0-main.c
test1_SOURCES=test1-main.c
test2_SOURCES=test2-main.c cr-test-utils.c cr-test-utils.h
test3_SOURCES=test3-main.c cr-test-utils.c cr-test-utils.h
test4_SOURCES=test4-main.c cr-test-utils.c cr-test-utils.h
test5_SOURCES=test5-main.c cr-test-utils.c cr-test-utils.h
test6_SOURCES=test6-main.c cr-test-utils.c cr-test-utils.h

croco_lib=$(top_builddir)/src/@CROCO_LIB@
LDADD=$(croco_lib)

INCLUDES=-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/parser \
	-I$(top_srcdir)/src/seleng \
	-I$(top_srcdir)/src/layeng \
	`pkg-config --cflags glib-2.0` \
	$(croco_layeng_needed_includes)

LDFLAGS=@GLIB2_LIBS@ @LIBXML2_LIBS@
AM_CFLAGS=-I. @LIBXML2_CFLAGS@ @GLIB2_CFLAGS@
