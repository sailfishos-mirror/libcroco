SUBDIRS = src
SUBDIRS += tests

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=libcroco.pc

extraincludedir=$(includedir)/@PACKAGE@
extrainclude_DATA= libcroco-config.h


bin_SCRIPTS=croco-config


EXTRA_DIST= croco-config.in libcroco.pc.in TODO Doxyfile HACKING \
docs/examples/cssom-example.c  docs/usage.txt tests/test-inputs/*.css \
tests/test-outputs/*.out

apidoc:
	if ! test -d docs/apis ; then  mkdir -p docs/apis ;  fi ;
	doxygen Doxyfile