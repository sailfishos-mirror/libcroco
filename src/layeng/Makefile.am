lib_LTLIBRARIES=libcrlayeng.la

crlayengincdir=$(includedir)/@PACKAGE@/layeng
crlayenginc_HEADERS= cr-box.h cr-box-view.h cr-lay-eng.h

#the layout engine files
if HAVE_LAYENG
LAYENG_SRCS= cr-box.c cr-box.h cr-box-view.c cr-box-view.h cr-lay-eng.c cr-lay-eng.h
endif

libcrlayeng_la_SOURCES=$(LAYENG_SRCS)
INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/src/parser \
-I$(top_srcdir)/src/seleng -I$(top_srcdir)/intl \
@GLIB2_CFLAGS@ @LIBXML2_CFLAGS@ @LIBGNOMEUI2_CFLAGS@

LDADD=$(top_srcdir)/src/parser/@CROCO_PARSER_LIB@

libcrlayeng_la_LDFLAGS=-version-info @LIBCRLAYENG_VERSION_INFO@ \
$(top_srcdir)/src/seleng/@CROCO_SELENG_LIB@ @LIBXML2_LIBS@ \
@LIBGNOMEUI2_LIBS@
