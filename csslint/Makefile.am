bin_PROGRAMS = csslint
man_MANS = csslint.1

csslint_SOURCES = csslint.c

LDADD=$(top_srcdir)/src/parser/libcroco.la

INCLUDES=-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/parser \
	-I$(top_srcdir)/src/seleng \
	-I$(top_srcdir)/src/layeng \
	`pkg-config --cflags glib-2.0`

LDFLAGS=@CROCO_LIBS@
AM_CFLAGS=-Wall -I. @CROCO_CFLAGS@

EXTRA_DIST = $(man_MANS)
