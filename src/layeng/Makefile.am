lib_LTLIBRARIES=libcrlayeng.la

crlayengincdir=$(includedir)/@PACKAGE@/layeng
crlayenginc_HEADERS= *.h

#the layout engine files
if HAVE_LAYENG
LAYENG_SRCS= cr-box.c cr-box-view.c cr-lay-eng.c
endif

libcrlayeng_la_SOURCES=$(LAYENG_SRCS)
INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/src/parser \
-I$(top_srcdir)/src/seleng -I$(top_srcdir)/intl \
@GLIB2_CFLAGS@ @LIBXML2_CFLAGS@ @LIBGNOMEUI2_CFLAGS@

LDADD=$(top_srcdir)/src/parser/@CROCO_PARSER_LIB@

libcrlayeng_la_LDFLAGS=-version-info @LIBCROCO_VERSION_INFO@ \
$(top_srcdir)/src/seleng/@CROCO_SELENG_LIB@ @LIBXML2_LIBS@ \
@LIBGNOMEUI2_LIBS@
