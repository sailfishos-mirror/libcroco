lib_LTLIBRARIES=libcroco.la


crparserincdir=$(includedir)/@PACKAGE@/parser
crparserinc_HEADERS=*.h

#the core parser files
SRCS=\
cr-utils.c \
cr-input.c \
cr-enc-handler.c \
cr-num.c \
cr-rgb.c \
cr-token.c \
cr-tknzr.c \
cr-term.c \
cr-attr-sel.c \
cr-pseudo.c \
cr-additional-sel.c \
cr-simple-sel.c \
cr-selector.c \
cr-doc-handler.c \
cr-parser.c \
cr-declaration.c \
cr-statement.c \
cr-stylesheet.c \
cr-cascade.c \
cr-om-parser.c \
*.h


libcroco_la_SOURCES= $(SRCS)

INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/intl `pkg-config --cflags glib-2.0` `pkg-config --cflags libxml-2.0` @LIBXML2_CFLAGS@
libcroco_la_LDFLAGS=-version-info @LIBCROCO_VERSION_INFO@ @LIBXML2_LIBS@
