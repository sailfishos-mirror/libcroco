/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * csslint.c : a small tester program for CSS2 input
 *
 * Copyright (C) 2002-2003 Gaël Chamoulaud <strider@freespiders.org>
 *                         Dodji Seketeli  <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "libcroco.h"

#include <glib.h>
#include <string.h>

/**
 *The options data structure.
 *The variable of this data structure are set
 *during the  parsing the command line by the
 *parse_command_line() function.
 */
struct Options
{
	gboolean show_version ;
	gboolean use_cssom ;

	gchar ** files_list ;
};

void
csslint_parse_cmd_line (int a_argc, char **a_argv, 
			struct Options *a_options);

static void
csslint_show_version (const char *name);

static void
csslint_usage (const char *name);

static enum CRStatus
csslint_cssom_parser_parse (guchar * a_file_uri);

/**
 *Parses the command line.
 *@param a_argc the argc parameter of the main routine.
 *@param the argv parameter of the main routine.
 *@param a_options out parameter the parsed options.
 */
void
csslint_parse_cmd_line (int a_argc, char **a_argv,
			struct Options *a_options)
{
	int i= 0 ;

	g_return_if_fail (a_options) ;
	        
        if (a_argc <= 1) 
        {
                csslint_usage(a_argv[0]);
        }
		
	for (i = 1 ; i < a_argc ; i++)
	{
		if (a_argv[i][0] != '-') break ;
		
		if ((!strcmp (a_argv[i], "-version")) || 
                    (!strcmp (a_argv[i], "--version")))
		{
			a_options->show_version = TRUE ;
		}
		if ((!strcmp (a_argv[i], "-cssom")) || 
                    (!strcmp (a_argv[i], "--cssom")))
		{
			a_options->use_cssom = TRUE ;
		}
	}
	
	if (i >= a_argc)
	{
		a_options->files_list = NULL ;
	}
	else
	{
		a_options->files_list = &a_argv[i] ;
	}
}

/**
 *Displays the version text.
 *@param a_argc the argc variable passed to the main function.
 *@param a_argv the argv variable passed to the main function.
 */
static void 
csslint_show_version (const char *name) 
{
    fprintf(stderr, "%s: using libcroco version %s\n", name, LIBCROCO_VERSION);
    fprintf(stderr, "   compiled with: ");
#ifdef CROCO_HAVE_LIBXML2
	fprintf(stderr, "LIBXML2 ");
#endif
#ifdef CROCO_SELENG_ENABLED
	fprintf(stderr, "SELENG ");
#endif
#ifdef CROCO_TESTS_ENABLED
	fprintf(stderr, "TESTS ");
#endif
    fprintf(stderr, "\n");
}

/**
 *Displays the usage text.
 *@param a_argc the argc variable passed to the main function.
 *@param a_argv the argv variable passed to the main function.
 */
static void 
csslint_usage(const char *name) 
{
    printf("Usage : %s [options] CSS2files ...\n", name);
    printf("\tParse the CSS2 files and output the result of the parsing\n");

	/* Listing of different options */
    printf("\t--version : display the version of the CSS2 library used\n");
	printf("\t--cssom : parse a CSS file and builds a CSS object model\n");

    printf("\nLibcroco project home page: http://www.freespiders.org/projects/libcroco\n");
    printf("To report bugs or get some help check: http://bugzilla.gnome.org/\n");
}

/**
 *The test of the cr_input_read_byte() method.
 *Reads the each byte of a_file_uri using the
 *cr_input_read_byte() method. Each byte is send to
 *stdout.
 *@param a_file_uri the file to read.
 *@return CR_OK upon successfull completion of the
 *function, an error code otherwise.
 */
static enum CRStatus
csslint_cssom_parser_parse (guchar * a_file_uri)
{
	enum CRStatus status = CR_OK ;
	CROMParser *parser = NULL ;
	CRStyleSheet *stylesheet = NULL ;
	
	g_return_val_if_fail (a_file_uri, CR_BAD_PARAM_ERROR) ;
		
	parser = cr_om_parser_new (NULL) ;
	status = cr_om_parser_parse_file (parser, 
                                          a_file_uri, CR_ASCII,
					  &stylesheet) ;
	if (status == CR_OK && stylesheet)
	{
		cr_stylesheet_dump (stylesheet, stdout) ;
		cr_stylesheet_destroy (stylesheet) ;
	}
	cr_om_parser_destroy (parser) ;

	return status ;
}

int 
main (int argc, char **argv) 
{
	struct Options options;
	enum CRStatus status = CR_OK;
	
        memset (&options, 0, sizeof (struct Options)) ;
        options.use_cssom = TRUE ;
	csslint_parse_cmd_line (argc, argv, &options);

	if (options.show_version == TRUE)
	{
		csslint_show_version(argv[0]);
		return 0;
	}

	if (options.use_cssom == TRUE)
	{
		if (options.files_list != NULL)
		{
			status = 
                                csslint_cssom_parser_parse 
                                (options.files_list[0]);
		} 
		else
		{
			csslint_show_version(argv[0]);
			return 0;
		}

		if (status == CR_OK)
		{
			g_print ("\nOK\n");
		}
		else
		{
			g_print ("\nKO\n");
		}
	}
    
	return 0;
}


