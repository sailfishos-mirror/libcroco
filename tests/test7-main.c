/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset:8 -*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms 
 * of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the 
 * GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/*
 *$Id$
 */

#include <string.h>
#include "libcroco.h"
#include "cr-test-utils.h"

/*
const guchar *gv_xmlbuf =
"<ARTICLE>"
"  <HEADLINE>Fredrick the Great meets Bach</HEADLINE>"
"  <AUTHOR>Johann Nikolaus Forkel</AUTHOR>"
"  <PARA>"
"    One evening, just as he was getting his"
"    <INSTRUMENT>flute</INSTRUMENT> ready and his"
"    musicians were assembled, an officer brought him a list of"
"    the strangers who had arrived."
"  </PARA>"
"</ARTICLE>"
*/
;

const guchar *gv_xmlbuf =
"<ARTICLE>"
"  <HEADLINE>Fredrick the Great meets Bach</HEADLINE>"
"  <AUTHOR>Johann Nikolaus Forkel</AUTHOR>"
"</ARTICLE>"
;
const guchar *gv_cssbuf =
"INSTRUMENT { display: inline}"
"ARTICLE, HEADLINE, AUTHOR, PARA { display: block ; position: static; background-color: rgb(255,0,0)}"
;

static enum CRStatus
test_layout_box (void)  ;

static gboolean
delete_event_cb (GtkWidget *a_widget, GdkEvent *a_event,
                 gpointer *a_user_data) ;


static gboolean
delete_event_cb (GtkWidget *a_widget, GdkEvent *a_event,
                 gpointer *a_user_data)
{
        gtk_main_quit () ;
        return FALSE ;
}

static enum CRStatus
test_layout_box (void)
{
	enum CRStatus status = CR_OK ;
	CRStyleSheet * sheet = NULL ;
	CRCascade *cascade = NULL ;
	CRLayEng *layout_engine = NULL ;
	xmlDoc *xml_doc = NULL ;
	gulong len = 0 ;
        CRBoxModel *box_model = NULL ;
        CRBoxView *box_view = NULL ;
        GtkWidget *window = NULL, *scroll = NULL ;

	len = strlen (gv_cssbuf) ;
	status = cr_om_parser_simply_parse_buf ((guchar *)gv_cssbuf, len,
						CR_UTF_8, &sheet) ;
	if (status != CR_OK || !sheet)
	{
		cr_utils_trace_info ("Could not parse css2 buf") ;
		status = CR_ERROR ;
		goto cleanup ;
	}

	len = strlen (gv_xmlbuf) ;
	xml_doc = xmlParseMemory (gv_xmlbuf, len) ;
	if (!xml_doc)
	{
		cr_utils_trace_info ("Could not parse xml buf") ;
		status = CR_ERROR ;
		goto cleanup ;
	}

	layout_engine = cr_lay_eng_new () ;
	if (!layout_engine)
	{
		cr_utils_trace_info 
			("Could not create the layout engine") ;
		cr_utils_trace_info 
			("The system is possibly out of memory") ;
		goto cleanup ;

	}

	cascade = cr_cascade_new (sheet, NULL, NULL) ;
	if (!cascade)
	{
		cr_utils_trace_info ("could not create the cascade") ;
		cr_utils_trace_info 
			("The system is possibly out of memory") ;
		goto cleanup ;
	}
        sheet = NULL ;

	status = cr_lay_eng_create_box_model (layout_engine,
                                              xml_doc, cascade,
                                              &box_model) ;
	if (status != CR_OK)
	{
		cr_utils_trace_info ("could not build the annotated doc") ;
		goto cleanup ;
	}

        if (box_model)
        {
                cr_lay_eng_layout_box_tree (layout_engine,
                                            ((CRBox*)box_model)->children) ;
                cr_box_dump_to_file ((CRBox*)box_model, 0, stdout) ;

                window = gtk_window_new (GTK_WINDOW_TOPLEVEL) ;
                if (!window) 
                {
                        cr_utils_trace_info ("System may be out of memory") ;
                        status = CR_ERROR ;
                        goto cleanup ;
                }
                gtk_window_set_title (GTK_WINDOW (window), 
                                      "Croco Renderer Test") ;
                gtk_window_set_policy (GTK_WINDOW (window), TRUE, TRUE, TRUE) ;
                gtk_widget_set_usize (window, 800, 600) ;
                g_signal_connect (G_OBJECT (window),
                                  "delete-event",
                                  G_CALLBACK (delete_event_cb),
                                  NULL) ;

                scroll = gtk_scrolled_window_new (NULL, NULL) ;
                if (!scroll)
                {
                        status = CR_ERROR ;
                        goto cleanup ;
                }
                gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                                GTK_POLICY_AUTOMATIC,
                                                GTK_POLICY_AUTOMATIC) ;

                box_view = cr_box_view_new (box_model) ;
                gtk_container_add (GTK_CONTAINER (window), scroll) ;
                gtk_container_add (GTK_CONTAINER (scroll), 
                                   GTK_WIDGET (box_view)) ;
                gtk_widget_show_all (window) ;
                gtk_main () ;

                return CR_OK ;
        }

 cleanup:

        if (scroll)
        {
                gtk_widget_destroy (scroll) ;
                scroll = NULL ;
        }

        if (window)
        {
                gtk_widget_destroy (window) ;
                window = NULL ;
        }

        if (box_view)
        {
                cr_box_view_destroy (GTK_OBJECT (box_view)) ;
                box_view = NULL ;
        }

	if (cascade)
	{
		cr_cascade_destroy (cascade) ;
		cascade = NULL ;
	}
	if (layout_engine)
	{
		cr_lay_eng_destroy (layout_engine) ;
		layout_engine = NULL ;
	}
	if (sheet)
	{
		cr_stylesheet_destroy (sheet) ;
		sheet = NULL ;
	}
	if (xml_doc)
	{
		xmlFreeDoc (xml_doc) ;
		xml_doc = NULL ;
	}
        if (box_model)
        {
                cr_box_destroy ((CRBox*)box_model) ;
                box_model = NULL ;
        }

	xmlCleanupParser () ;
	return status ;
}

int
main (int argc, char **argv)
{
	enum CRStatus status = CR_OK ;

        cr_lay_eng_init (argc, argv) ;

	status = test_layout_box () ;

	if (status != CR_OK)
	{
		g_print ("\nKO\n") ;
	}
	return 0 ;
}
