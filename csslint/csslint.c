/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * csslint.c : a small tester program for CSS2 input
 *
 * Copyright (C) 2002-2003 Gaël Chamoulaud, Dodji Seketeli.
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
#include <libxml/xpath.h>

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
        gboolean display_help ;
        gboolean evaluate ;
        gchar *author_sheet_path ;
        gchar *user_sheet_path ;
        gchar *ua_sheet_path ;
        gchar *xml_path ;
        gchar *xpath ;
	gchar ** css_files_list ;
};

static void
parse_cmd_line (int a_argc, char **a_argv, 
                struct Options *a_options);

static void
display_version (void);

static void
display_usage (void);

static enum CRStatus
cssom_parse (guchar * a_file_uri);

static enum CRStatus
get_and_dump_node_style (xmlNode *a_node,
                         CRSelEng *a_sel_eng,
                         CRCascade *a_cascade) ;
static enum CRStatus
evaluate_selectors (gchar *a_xml_path,
                    gchar *a_author_sheet_path,
                    gchar *a_user_sheet_path,
                    gchar *a_ua_sheet_path,
                    gchar *a_xpath) ;

/**
 *Parses the command line.
 *@param a_argc the argc parameter of the main routine.
 *@param the argv parameter of the main routine.
 *@param a_options out parameter the parsed options.
 */
static void
parse_cmd_line (int a_argc, char **a_argv,
                struct Options *a_options)
{
	int i= 0 ;

	g_return_if_fail (a_options) ;
	        
        if (a_argc <= 1) 
        {
                display_usage();
        }
		
	for (i = 1 ; i < a_argc ; i++)
	{
		if (a_argv[i][0] != '-') break ;
		
		if ((!strcmp (a_argv[i], "-version")) || 
                    (!strcmp (a_argv[i], "-v")))
		{
			a_options->show_version = TRUE ;
		} 
                else if ( !strcmp (a_argv[i], "--evaluate") ||
                        !strcmp (a_argv[i], "-e"))
                {
                        gchar *xml_doc_path = NULL,
                                *author_sheet_path = NULL,
                                *user_sheet_path = NULL,
                                *ua_sheet_path = NULL ;

                        for (i++ ; i < a_argc ; i++)
                        {
                                if (!strcmp (a_argv[i], "--author-sheet"))
                                {
                                        if (author_sheet_path)
                                        {
                                                display_usage () ;
                                                exit (-1);
                                        }
                                        author_sheet_path = a_argv[i] ;
                                        i++ ;
                                        if (i >= a_argc || a_argv[i][0] == '-')
                                        {
                                                g_print ("--author-sheet should be followed by a path to the sheet\n") ;
                                                display_usage () ;
                                                exit (-1);
                                        }
                                        a_options->author_sheet_path = a_argv[i] ;
                                }
                                else if (!strcmp (a_argv[i], "--user-sheet"))
                                {
                                        if (user_sheet_path)
                                        {
                                                display_usage () ;
                                                exit (-1);
                                        }
                                        user_sheet_path = a_argv[i] ;
                                        i++ ;
                                        if (i >= a_argc || a_argv[i][0] == '-')
                                        {
                                                g_print ("--user-sheet should be followed by a path to the sheet\n") ;
                                                display_usage () ;
                                                exit (-1);
                                        }
                                        a_options->user_sheet_path = a_argv[i] ;
                                }
                                else if (!strcmp (a_argv[i], "--ua-sheet"))
                                {
                                        if (ua_sheet_path)
                                        {
                                                display_usage () ;
                                                exit (-1);
                                        }
                                        ua_sheet_path = a_argv[i] ;
                                        if (i >= a_argc || a_argv[i][0] == '-')
                                        {
                                                g_print ("--ua-sheet should be followed by a path to the sheet\n") ;
                                                display_usage () ;
                                                exit (-1);
                                        }
                                        a_options->ua_sheet_path = a_argv[i] ;
                                }
                                else if (!strcmp (a_argv[i], "--xml"))
                                {
                                        i++ ;
                                        if (i >= a_argc || a_argv[i][0] == '-')
                                        {
                                                g_print ("--xml should be followed by a path to the xml document\n") ;
                                                display_usage () ;
                                                exit (-1);
                                        }
                                        a_options->xml_path = a_argv[i] ;
                                }
                                else if (!strcmp (a_argv[i], "--xpath"))
                                {
                                        i++ ;
                                        if (i >= a_argc || a_argv[i][0] == '-')
                                        {
                                                g_print ("--xpath should be followed by an xpath expresion\n") ;
                                                display_usage () ;
                                                exit (-1);
                                        }
                                        a_options->xpath = a_argv[i] ;
                                }
                                else
                                {
                                        break ;
                                }
                        }
                        if (!author_sheet_path  && !user_sheet_path && !ua_sheet_path)
                        {
                                g_print ("Error: you must specify at least one stylesheet\n") ;
                                display_usage () ;
                                exit (-1) ;
                                
                        }
                        if (!a_options->xpath)
                        {
                                g_printerr ("Error: you must specify an xpath expression using the --xpath option\n") ;
                                display_usage () ;
                                exit (-1) ;
                                
                        }
                        a_options->evaluate = TRUE ;
                } else if (!strcmp (a_argv[i], "--help") ||
                           !strcmp (a_argv[i], "-h"))
                {
                        a_options->display_help = TRUE ;
                }
                else
                {
                        display_usage () ;
                        exit (-1);
                }
	}
	
	if (i >= a_argc)
	{
		a_options->css_files_list = NULL ;
	}
	else
	{
                if (a_argv[i][0] == '-')
                {
                        display_usage () ;
                        exit (-1) ;
                }
		a_options->css_files_list = &a_argv[i] ;
	}
}

/**
 *Displays the version text.
 *@param a_argc the argc variable passed to the main function.
 *@param a_argv the argv variable passed to the main function.
 */
static void 
display_version (void) 
{
        g_print ("%s\n", LIBCROCO_VERSION) ;
}

/**
 *Displays the usage text.
 *@param a_argc the argc variable passed to the main function.
 *@param a_argv the argv variable passed to the main function.
 */
static void 
display_usage (void)
{
    g_print ("Usage: csslint <path to a css file>\n");
    g_print ("\t| csslint -v|--version\n") ;
    g_print ("\t| csslint <--evaluate | -e> [--author-sheet <path> --user-sheet <path> --ua-sheet <path>\n] --xml <path> --xpath <xpath expression>") ;
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
cssom_parse (guchar * a_file_uri)
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
                g_print ("\n") ;
		cr_stylesheet_destroy (stylesheet) ;
	}
	cr_om_parser_destroy (parser) ;

	return status ;
}

static enum CRStatus
get_and_dump_node_style (xmlNode *a_node,
                         CRSelEng *a_sel_eng,
                         CRCascade *a_cascade)
{
        CRPropList *prop_list = NULL, *pair = NULL, *prev_pair = NULL ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_node && a_sel_eng && a_cascade,
                              CR_BAD_PARAM_ERROR) ;

        status = cr_sel_eng_get_matched_properties_from_cascade 
                (a_sel_eng, a_cascade, a_node, &prop_list) ;
        if (status != CR_OK)
        {
                g_printerr ("Error: unable to run the selection engine\n") ;
                return CR_OK ;
        }
        g_print ("Properties of xml element %s are:\n", 
                 a_node->name) ;
        for (pair = prop_list ; pair ; 
             pair = cr_prop_list_get_next (pair))
        {
                CRDeclaration *decl = NULL ;

                cr_prop_list_get_decl (pair, &decl) ;
                if (decl)
                {
                        prev_pair = cr_prop_list_get_prev (pair) ;
                        if (prev_pair)
                        {
                                g_print ("\n") ;
                                prev_pair = NULL ;
                        }
                        cr_declaration_dump_one 
                                (decl, stdout, 2) ;
                        decl = NULL ;
                }
        }
        g_print ("\n=====================\n\n") ;

        if (prop_list)
        {
                cr_prop_list_destroy (prop_list) ;
                prop_list = NULL ;
        }

        return CR_OK ;
}

static enum CRStatus
evaluate_selectors (gchar *a_xml_path,
                    gchar *a_author_sheet_path,
                    gchar *a_user_sheet_path,
                    gchar *a_ua_sheet_path,
                    gchar *a_xpath)
{
        CRSelEng *sel_eng = NULL ;
        xmlDoc *xml_doc = NULL ;
        xmlXPathContext *xpath_context = NULL ;
        xmlXPathObject *xpath_object = NULL ;
        CRStyleSheet *author_sheet = NULL, *user_sheet = NULL, *ua_sheet = NULL ;
        CRCascade *cascade = NULL ;
        xmlNode *cur_node = NULL ;
        gint i = 0 ;
        enum CRStatus status = CR_OK ;

        g_return_val_if_fail (a_xml_path && a_xpath, CR_BAD_PARAM_ERROR) ;

        xml_doc = xmlParseFile (a_xml_path) ;
        if (!xml_doc)
        {
                g_printerr ("Error: Could not parse file %s\n", a_xml_path) ;
                return CR_ERROR ;
        }
        if (a_author_sheet_path)
        {
                status = cr_om_parser_simply_parse_file 
                        (a_author_sheet_path, CR_ASCII, &author_sheet) ;
                if (!author_sheet)
                {
                        g_printerr ("Error: Could not parse author sheet\n") ;
                }
        }
        if (a_user_sheet_path)
        {
                status = cr_om_parser_simply_parse_file 
                        (a_user_sheet_path, CR_ASCII, &user_sheet) ;
                if (!user_sheet)
                {
                        g_printerr ("Error: Could not parse author sheet\n") ;
                }
        }
        if (a_ua_sheet_path)
        {
                status = cr_om_parser_simply_parse_file 
                        (a_ua_sheet_path, CR_ASCII, &ua_sheet) ;
                if (!ua_sheet)
                {
                        g_printerr ("Error: Could not parse ua sheet\n") ;
                }
        }
        cascade = cr_cascade_new (author_sheet, user_sheet, 
                                  ua_sheet) ;
        if (!cascade)
        {
                g_printerr ("Could not instanciate the cascade\n") ;
                return CR_ERROR ;
        }
        sel_eng = cr_sel_eng_new () ;
        if (!sel_eng)
        {
                g_printerr ("Error: Could not instanciate the selection engine\n") ;
                return CR_ERROR ;
        }
        xpath_context = xmlXPathNewContext (xml_doc) ;
        if (!xpath_context)
        {
                g_printerr ("Error: Could not instanciate the xpath context\n") ;
                return CR_ERROR ;
        }
        xpath_object = xmlXPathEvalExpression (a_xpath, 
                                               xpath_context) ;
        if (!xpath_object)
        {
                g_printerr ("Error: Could not evaluate xpath expression\n") ;
                return CR_ERROR ;
        }
        if (xpath_object->type != XPATH_NODESET || !xpath_object->nodesetval)
        {
                g_printerr ("Error: xpath does not evalualuate to a node set\n") ;
                return CR_ERROR ;
        }
        
        for (i = 0; i < xpath_object->nodesetval->nodeNr ;
             i++)
        {
                cur_node = xpath_object->nodesetval->nodeTab[i] ;
                if (cur_node->type == XML_ELEMENT_NODE) 
                {
                        status = get_and_dump_node_style (cur_node, sel_eng,
                                                          cascade) ;
                }
        }
        return CR_OK ;
}

int 
main (int argc, char **argv) 
{
	struct Options options;
	enum CRStatus status = CR_OK;
	
        memset (&options, 0, sizeof (struct Options)) ;
        options.use_cssom = TRUE ;
	parse_cmd_line (argc, argv, &options);

	if (options.show_version == TRUE)
	{
		display_version ();
		return 0;
	}

        if (options.display_help == TRUE) 
        {
                display_usage () ;
                return 0 ;
        }
	if (options.use_cssom == TRUE)
	{
                if (options.evaluate == TRUE)
		{                        
                        status = evaluate_selectors 
                                (options.xml_path,
                                 options.author_sheet_path,
                                 options.user_sheet_path,
                                 options.ua_sheet_path,
                                 options.xpath) ;
		}
                else if (options.css_files_list != NULL)
		{
			status = 
                                cssom_parse 
                                (options.css_files_list[0]);
		} 
	}
    
	return 0;
}


