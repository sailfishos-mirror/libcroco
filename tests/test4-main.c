/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
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

/*
 *$Id$
 */

#include "cr-test-utils.h"
#include "libcroco.h"


/**
 *@file
 *Some test facilities for the #CROMParser class.
 */

CRDocHandler * gv_test_handler = {0} ;

const guchar * gv_decl=
"toto: tutu, tata" ;

static void 
display_help (char *prg_name) ;

static void 
display_about (char *prg_name) ;

static enum CRStatus
test_cr_parser_parse (guchar * a_file_uri) ;

/**
 *Displays the usage of the test
 *facility.
 *@param a_argc the argc variable passed to the main function.
 *@param a_argv the argv variable passed to the main function.
 */
static void 
display_help (char *prg_name)
{
        g_print ("\n\n") ;
        g_print ("usage: %s <file-to-parse>\n", prg_name) ;
        g_print ("\t <file-to-parse>: the file to parse\n") ;
        g_print ("\n\n") ;
        g_print ("Tests the cr_parser_parse () method.\n") ;
        g_print ("Tests the parsing following the css core syntax\n") ;
        g_print ("Returns OK if the status is CR_OK, KO otherwise\n") ;
        g_print ("\n\n") ;
}

/**
 *Displays the about text.
 *@param a_argc the argc variable passed to the main function.
 *@param a_argv the argv variable passed to the main function.
 */
static void 
display_about (char *prg_name)
{
        g_print ("\n\n") ;
        g_print ("%s is a libcroco CROMParser class test program.\n", 
		 prg_name) ;
	g_print ("%s Parses a file and builds a CSS object model", 
		 prg_name) ;
        g_print ("It should run on GNU compliants systems.\n") ;
        g_print ("\n\n") ;
        g_print ("Initial author: Dodji Seketeli <dodji@seketeli.org>.\n") ;
        g_print ("\n\n") ;
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
test_cr_parser_parse (guchar * a_file_uri)
{
        enum CRStatus status = CR_OK ;
        CROMParser *parser = NULL ;
	CRStyleSheet *stylesheet = NULL ;

        g_return_val_if_fail (a_file_uri, CR_BAD_PARAM_ERROR) ;

	parser = cr_om_parser_new (NULL) ;
	status = cr_om_parser_parse_file (parser, a_file_uri, CR_ASCII,
					  &stylesheet) ;
	if (status == CR_OK && stylesheet)
	{
		cr_stylesheet_dump (stylesheet, stdout) ;
		cr_stylesheet_destroy (stylesheet) ;
	}
	cr_om_parser_destroy (parser) ;

        return status ;
}

static enum CRStatus
test_cr_declaration_parse (void)
{
        guchar * tmp_str = NULL ;
        CRDeclaration * decl = NULL ;

        decl = cr_declaration_parse (NULL, gv_decl,
                                     CR_UTF_8) ;
        tmp_str = cr_declaration_to_string (decl, 2) ;

        if (decl)
        {
                cr_declaration_destroy (decl) ;
                return CR_OK ;
        }

        if (tmp_str)
        {
                g_free (tmp_str) ;
                tmp_str = NULL ;
        }

        return CR_ERROR ;
}

/**
 *The entry point of the testing routine.
 */
int
main (int argc, char ** argv)
{
        struct Options options ;
        enum CRStatus status = CR_OK ;

        status = test_cr_declaration_parse () ;

        if (status != CR_OK)
        {
                g_print ("\nKO\n") ;
                return 0 ;
        }
                
        cr_test_utils_parse_cmd_line (argc, argv, &options) ;

        if (options.display_help == TRUE)
        {
                display_help (argv[0]) ;
                return 0 ;
        }

        if (options.display_about == TRUE)
        {
                display_about (argv[0]) ;
                return 0 ;
        }

        if (options.files_list == NULL)
        {
                display_help (argv[0]) ;
                return 0 ;
        }
        
        status = test_cr_parser_parse (options.files_list[0]) ;

        
        if (status != CR_OK)
        {
                g_print ("\nKO\n") ;
        }

	return 0 ;
}
