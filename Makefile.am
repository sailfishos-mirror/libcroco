SUBDIRS = src csslint docs
SUBDIRS += tests

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=libcroco.pc

extraincludedir=$(includedir)/@PACKAGE@

bin_SCRIPTS=croco-config

man_MANS = croco-config.1

EXTRA_DIST= croco-config.in libcroco.pc libcroco.spec TODO Doxyfile \
HACKING COPYING.LIB COPYING  libcroco.spec tests/testctl $(man_MANS)

cleantar:
	@(rm -f libcroco*.tar.gz)
	@(rm -rf $(top_srcdir)/rpmbuildroot)

rpm: cleantar
	@(mkdir -p $(top_srcdir)/rpmbuildroot ; $(MAKE) dist && rpmbuild  -ta --buildroot $(top_srcdir)/rpmbuildroot $(distdir).tar.gz)

nr-tests:
	if test -x $(top_srcdir)/tests/testctl ; then $(top_srcdir)/tests/testctl mkcleanup ; $(top_srcdir)/tests/testctl run ; fi 

apidoc:
	if ! test -d docs/apis ; then  mkdir -p docs/apis ;  fi ;
	doxygen Doxyfile
