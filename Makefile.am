SUBDIRS = src csslint
SUBDIRS += tests

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=libcroco.pc

extraincludedir=$(includedir)/@PACKAGE@
extrainclude_DATA= libcroco-config.h


bin_SCRIPTS=croco-config

man_MANS = croco-config.1

EXTRA_DIST= croco-config.in libcroco.pc libcroco.spec TODO Doxyfile \
HACKING COPYING.LIB COPYING docs/examples/cssom-example.c  \
docs/usage.txt  tests/test-inputs/*.css \
tests/test-output-refs/*.out $(man_MANS)

cleantar:
	@(rm -f libcroco*.tar.gz)
	@(rm -rf $(top_srcdir)/rpmbuildroot)

rpm: cleantar
	@(mkdir -p $(top_srcdir)/rpmbuildroot ; $(MAKE) dist && rpmbuild  -ta --buildroot $(top_srcdir)/rpmbuildroot $(distdir).tar.gz)

apidoc:
	if ! test -d docs/apis ; then  mkdir -p docs/apis ;  fi ;
	doxygen Doxyfile
