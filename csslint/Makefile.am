bin_PROGRAMS = csslint
man_MANS = csslint.1

csslint_SOURCES = csslint.c

croco_parser_lib=$(top_builddir)/src/parser/@CROCO_PARSER_LIB@

if HAVE_SELENG
croco_seleng_lib=$(top_builddir)/src/seleng/@CROCO_SELENG_LIB@
croco_seleng_cflags=-I$(top_srcdir)/src/seleng @LIBXML2_CFLAGS@
endif

if HAVE_LAYENG
croco_layeng_lib=$(top_builddir)/src/layeng/@CROCO_LAYENG_LIB@
croco_layeng_cflags=-I$(top_srcdir)/src/layeng @LIBGNOMEUI2_CFLAGS@
endif

LDADD=$(croco_layeng_lib) $(croco_seleng_lib) $(croco_parser_lib) 

INCLUDES=-I$(top_srcdir)/intl \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/parser \
	@GLIB2_CFLAGS@ \
	$(croco_seleng_cflags) \
	$(croco_layeng_cflags)

AM_CFLAGS=$(INCLUDES)

LDFLAGS=@GLIB2_LIBS@

EXTRA_DIST = $(man_MANS)
