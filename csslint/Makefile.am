bin_PROGRAMS = csslint
man_MANS = csslint.1

csslint_SOURCES = csslint.c

LDADD=$(top_srcdir)/src/libcroco.la

INCLUDES=-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src \
	`pkg-config --cflags glib-2.0`

LDFLAGS=`pkg-config --libs glib-2.0` @LIBXML2_LIBS@
AM_CFLAGS=-Wall -I. @LIBXML2_CFLAGS@
