lib_LTLIBRARIES=libcrseleng.la

crselengincdir=$(includedir)/@PACKAGE@/seleng
crselenginc_HEADERS= cr-fonts.h cr-sel-eng.h cr-style.h

#the selection engine files
if HAVE_SELENG
SELENG_SRCS= cr-style.c cr-style.h cr-sel-eng.c cr-sel-eng.h cr-fonts.c cr-fonts.h
endif

libcrseleng_la_SOURCES=$(SELENG_SRCS)
INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/src/parser -I$(top_srcdir)/intl \
@GLIB2_CFLAGS@ @LIBXML2_CFLAGS@

LDADD=$(top_srcdir)/src/parser/libcroco.la

libcrseleng_la_LDFLAGS=-version-info @LIBCRSELENG_VERSION_INFO@ @LIBXML2_LIBS@
