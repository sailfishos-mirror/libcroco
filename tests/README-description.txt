This file describes the tests present in the libcroro tests directory.
=====================================================================

Initial author: Dodji Seketeli <dodji@seketeli.org>

Each entry of this file is orgnised as follows:

"""""""""""""""""""""""""""""""""""""""""""""""""
binary: name-of-the-test-binary-file

source-file: name-of-the-test-source-file

purpose: purpose of the test.

description: description of the test
"""""""""""""""""""""""""""""""""""""""""""""""""

And, in the words of RMS, happy hacking !

      Dodji
##################################################################


"""""""""""""""""""""""""""""""""""""""""""""""""
binary: test0

source-file: test0-main.c

purpose: Tests the cr_input_read_byte() method of the CRInput class.

description: opens the file located at the path in parameter and
loads it into an input stream using the cr_input_new_from_uri() 
method of the CRInput class.
Then, reads the bytes from the input stream using the cr_input_read_byte()
method and dumps each byte read to stdout.
Note that the input file must be encoded in utf8.

"""""""""""""""""""""""""""""""""""""""""""""""""


"""""""""""""""""""""""""""""""""""""""""""""""""
binary: test1

source-file: test1-main.c

purpose: Test the read_char() method of the CRInput class.

description: opens the file located at the path in parameter and
loads it into an input stream using the cr_input_new_from_uri() 
method of the CRInput class.
Then, reads characters from the input stream using the cr_input_read_char()
method and dumps each character on stdout.
Note that the input file must be encoded in utf8.
"""""""""""""""""""""""""""""""""""""""""""""""""

"""""""""""""""""""""""""""""""""""""""""""""""""
binary: test2

source-file: test2-main.c

purpose: tests the sac api of the parser.

description: it loads the file located at the path given
in argument, instanciates a sac parser, set document
handlers and parses the file. 
The doc handlers simply dump the name name of the callback
which is being called and some basic additional information.
"""""""""""""""""""""""""""""""""""""""""""""""""

"""""""""""""""""""""""""""""""""""""""""""""""""
binary: test3

source-file: test3-main.c

purpose: Test the SAC api core css grammar parsing

description: it loads the file located at the path given
in argument, instanciates a sac parser, set document
handlers and parses the file using the css2 core  syntax.
The doc handlers simply dump the name name of the callback
which is being called and some basic additional information.
Here, the goal is that nothing is dump on screen because
during the parsing using the core grammar syntax, no callback
should be called.
"""""""""""""""""""""""""""""""""""""""""""""""""

"""""""""""""""""""""""""""""""""""""""""""""""""
binary: test4

source-file: test4-main.c

purpose: tests the cssom parser interface (cr-om-parser.c)

description: parses the file located at the path given in
parameter and dumps the result of the parsing (the object model)
on stdout.
"""""""""""""""""""""""""""""""""""""""""""""""""
