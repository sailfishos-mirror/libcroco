lib_LTLIBRARIES=libcrseleng.la

crselengincdir=$(includedir)/@PACKAGE@/seleng
crselenginc_HEADERS= *.h

#the selection engine files
if HAVE_SELENG
SELENG_SRCS= cr-sel-eng.c
endif

libcrseleng_la_SOURCES=$(SELENG_SRCS)
INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/src/parser -I$(top_srcdir)/intl \
`pkg-config --cflags glib-2.0` `pkg-config --cflags libxml-2.0` @LIBXML2_CFLAGS@

LDADD=$(top_srcdir)/src/parser/libcroco.la

libcrseleng_la_LDFLAGS=-version-info @LIBCROCO_VERSION_INFO@ @LIBXML2_LIBS@
