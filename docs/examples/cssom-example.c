/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset:8 -*- */

/**
 *This is an example that shows how to use
 *the libcroco css2 parsing library.
 *
 *To compile it using gcc, type
 *
 *gcc `croco-config --cflags`  `croco-config --libs` -o cssom-example cssom-example.c
 *
 *Prior to that, you must have compiled and installed libcroco, of course.
 *
 *@author Dodji Seketeli <dodji@seketeli.org>
 */

/*
 *$Id$
 */

#include <string.h>
#include <libcroco.h>

/**
 *Displays the usage of this program.
 *@param prg_name the name of the program.
 */
void
display_usage (char *prg_name)
{
	printf ("\nusage: %s [options] <css file to parse>\n\n", 
                prg_name) ;
	printf ("\twhere options are:\n") ;
	printf ("\t--help|h\tdisplays this help\n") ;
	printf ("\n") ;
}


/**
 *Main entry point.
 *This function illustrates a way to use libcroco.
 *In this example, we will use a CSS Object Model parser
 *to parse a cascading style sheet and dump what we have parsed on stdout.
 *
 *The goal of the object model parser is to parse a css and build
 *an abstract tree out of it. One can then walk the tree to perform
 *whatever action he wants. In our case, the function used to
 *dump the tree on stdout will walk the tree and dump each one of its
 *components (a.k.a. css statements).
 */
int
main (int argc, char **argv)
{
	short i = 0 ;
	enum CRStatus status = CR_OK ;
	CROMParser *parser = NULL ;
	CRStyleSheet *stylesheet = NULL ;

	/*first parse command line arguments*/
	for (i = 1 ; i < argc ; i++)
	{
		if (argv[i][0] != '-')
			break ;
		if (!strcmp (argv[i],"--help") 
		    || !strcmp (argv[i], "-h"))
		{
			display_usage (argv[0]) ;
			return 0 ;
		}
		else
		{
			display_usage (argv[0]) ;
			return 0 ;
		}
	}
	if (i >= argc)
	{
		display_usage (argv[0]) ;
		return 0;
	}

	/*
	 *instanciate a new css object model parser.
	 */	
	parser = cr_om_parser_new (NULL) ;

	if (!parser)
	{
		fprintf (stderr, 
			 "Arrgh!!! Couldn't instanciate the parser.\n");
		return -1 ;
	}

	status = cr_om_parser_parse_file (parser, 
					  argv[i] /*the style sheet 
						    file path*/, 
					  CR_ASCII /*the encoding*/,
					  &stylesheet) ;

	if (status == CR_OK && stylesheet)
	{
		/*
		 *everything went well, 
		 *so dump the stylesheet on stdout.
		 */
		cr_stylesheet_dump (stylesheet, stdout) ;
	}

	if (stylesheet)
	{
		/*
		 *free the the memory used to hold the css
		 *object model
		 */
		cr_stylesheet_destroy (stylesheet) ;
		stylesheet = NULL ;
	}

	/*free the memory used by the parser*/
	cr_om_parser_destroy (parser) ;
	parser = NULL ;

	return 0 ;
}
