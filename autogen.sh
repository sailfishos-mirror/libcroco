#!/bin/sh

#Author: Dodji Seketeli.


echo "autogen.sh: Running aclocal ..."
aclocal
echo "autogen.sh: aclocal: done"

echo "autogen.sh: Running autoconf -f ..."
autoconf
echo "autogen.sh: autoconf: done"

echo "autogen.sh: Running automake -a -c ..."
automake --gnu -a -c 
echo "autogen.sh: automake: done"

echo "autogen.sh: Running configure ..."
./configure $@
echo "autogen.sh: configure: done"
