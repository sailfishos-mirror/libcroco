bin_PROGRAMS = csslint
man_MANS = csslint.1

csslint_SOURCES = csslint.c

croco_parser_lib=$(top_builddir)/src/parser/@CROCO_PARSER_LIB@

if HAVE_SELENG
croco_seleng_lib=$(top_builddir)/src/seleng/@CROCO_SELENG_LIB@
endif

if HAVE_LAYENG
croco_layeng_lib=$(top_builddir)/src/layeng/@CROCO_LAYENG_LIB@
endif

LDADD=@GLIB2_LIBS@ $(croco_parser_lib) $(croco_seleng_lib) $(croco_layeng_lib)

INCLUDES=-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/parser \
	-I$(top_srcdir)/src/seleng \
	-I$(top_srcdir)/src/layeng \
	@GLIB2_CFLAGS@ @LIBXML2_CFLAGS@ @LIBGNOMEUI2_CFLAGS@

AM_CFLAGS=$(INCLUDES)


EXTRA_DIST = $(man_MANS)
