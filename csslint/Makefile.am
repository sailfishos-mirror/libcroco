bin_PROGRAMS = csslint
man_MANS = csslint.1

csslint_SOURCES = csslint.c

croco_lib=$(top_builddir)/src/@CROCO_LIB@
LDADD=$(croco_lib)

INCLUDES=-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src \
	@GLIB2_CFLAGS@ \
	@LIBXML2_CFLAGS@

AM_CFLAGS=$(INCLUDES)

LDFLAGS=@GLIB2_LIBS@ @LIBXML2_LIBS@

EXTRA_DIST = $(man_MANS)
